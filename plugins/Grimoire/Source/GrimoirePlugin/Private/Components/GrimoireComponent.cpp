#include "Components/GrimoireComponent.h"
#include "Model/HeartGraph.h"
#include "Spells/SpellNode.h"
#include "Spells/MagicNode.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UGrimoireComponent::UGrimoireComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentMana = 100.0f;
    MaxMana = 100.0f;
	ManaRegenRate = 5.0f;  
    BaseDamage = 10.0f;
    SetIsReplicatedByDefault(true);

    AvailableNodeClasses.Empty();
    OwnedNodes.Empty();
    ActiveSpells.Empty();
    SpellCooldowns.Empty();
}

void UGrimoireComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGrimoireComponent, AvailableNodeClasses);
    DOREPLIFETIME(UGrimoireComponent, ActiveSpells);
    DOREPLIFETIME(UGrimoireComponent, CurrentMana);
}

void UGrimoireComponent::BeginPlay()
{
    Super::BeginPlay();
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("GrimoireComponent has no owner"));
        return;
    }

    // Setup Ability System Component
    AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No AbilitySystemComponent found on owner, creating one"));
        AbilitySystemComponent = NewObject<UAbilitySystemComponent>(Owner, TEXT("GrimoireAbilitySystem"));
        Owner->AddInstanceComponent(AbilitySystemComponent);
        AbilitySystemComponent->RegisterComponent();
    }

    // Setup Enhanced Input if this is a player
    if (APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (InputContext)
            {
                Subsystem->AddMappingContext(InputContext, 0);
                UE_LOG(LogTemp, Log, TEXT("Added input mapping context for Grimoire"));
            }
        }
    }

    // Initialize with basic nodes for testing
    if (GetOwner()->HasAuthority())
    {
        AddSpellNode(UMagicNode::StaticClass());
        CreateSpell(TEXT("TestFireSpell"));
    }

    UE_LOG(LogTemp, Log, TEXT("GrimoireComponent initialized for %s"), *Owner->GetName());
}

void UGrimoireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // Regenerate mana
    if (CurrentMana < MaxMana)
    {
        CurrentMana = FMath::Min(MaxMana, CurrentMana + ManaRegenRate * DeltaTime);
    }

    // Update spell cooldowns
    TArray<FName> CooledDownSpells;
    for (auto& CooldownPair : SpellCooldowns)
    {
        CooldownPair.Value -= DeltaTime;
        if (CooldownPair.Value <= 0.0f)
        {
            CooledDownSpells.Add(CooldownPair.Key);
        }
    }

    // Remove expired cooldowns
    for (const FName& SpellName : CooledDownSpells)
    {
        SpellCooldowns.Remove(SpellName);
    }
}

void UGrimoireComponent::OnRep_CurrentMana()
{
    OnManaChanged.Broadcast(CurrentMana);
    UE_LOG(LogTemp, Log, TEXT("Mana replicated: %f"), CurrentMana);
}

void UGrimoireComponent::AddSpellNode(TSubclassOf<USpellNode> SpellNodeClass)
{
    if (!SpellNodeClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add null spell node class"));
        return;
    }

    // Add to available classes if not already present
    AvailableNodeClasses.AddUnique(SpellNodeClass);
    
    // Create an instance for immediate use
    USpellNode* NewNode = CreateNodeInstance(SpellNodeClass);
    if (NewNode)
    {
        OwnedNodes.Add(NewNode);
        OnNodeAdded.Broadcast(NewNode);
        UE_LOG(LogTemp, Log, TEXT("Added spell node: %s"), *NewNode->NodeName);
    }
}

void UGrimoireComponent::RemoveSpellNode(USpellNode* SpellNode)
{
    if (!SpellNode) return;
   
    OwnedNodes.Remove(SpellNode);
    UE_LOG(LogTemp, Log, TEXT("Removed spell node: %s"), *SpellNode->NodeName);
}

USpellNode* UGrimoireComponent::CreateNodeInstance(TSubclassOf<USpellNode> NodeClass)
{
    if (!NodeClass)
    {
        return nullptr;
    }

    USpellNode* NewNode = NewObject<USpellNode>(this, NodeClass);
    if (NewNode)
    {
        // Initialize the node
        NewNode->PostInitProperties();
        UE_LOG(LogTemp, Log, TEXT("Created node instance: %s"), *NewNode->NodeName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create node instance of class: %s"), *NodeClass->GetName());
    }

    return NewNode;
}

UHeartGraph* UGrimoireComponent::GetSpellGraph(FName SpellName)
{
    if (FSpellDefinition* SpellDef = ActiveSpells.Find(SpellName))
    {
        return SpellDef->SpellGraph;
    }
    return nullptr;
}

void UGrimoireComponent::AddSpellGraph(FName GraphName)
{
    UHeartGraph* NewGraph = NewObject<UHeartGraph>(this);
    ActiveSpellGraphs.Add(GraphName, NewGraph);
    USpellNode* Magic = NewObject<UMagicNode>(NewGraph);
    NewGraph->AddNode(Magic);
    NewGraph->SetRootNode(Magic); 
}

void UGrimoireComponent::RemoveSpellGraph(FName GraphName)
{
    ActiveSpellGraphs.Remove(GraphName);
}

void UGrimoireComponent::CreateSpell(FName SpellName)
{
    if (SpellName == NAME_None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create spell with empty name"));
        return;
    }

    if (ActiveSpells.Contains(SpellName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Spell %s already exists"), *SpellName.ToString());
        return;
    }

    // Create new Heart Graph for the spell
    UHeartGraph* NewGraph = NewObject<UHeartGraph>(this, UHeartGraph::StaticClass(), 
        *FString::Printf(TEXT("SpellGraph_%s"), *SpellName.ToString()));
    
    if (!NewGraph)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create spell graph for %s"), *SpellName.ToString());
        return;
    }

    // Create spell definition
    FSpellDefinition SpellDef;
    SpellDef.SpellName = SpellName;
    SpellDef.SpellGraph = NewGraph;
    SpellDef.ManaCost = 0.0f; // Will be calculated from nodes
    SpellDef.Cooldown = 1.0f;
    SpellDef.InputBinding = EGWTAbilityInputID::None;

    ActiveSpells.Add(SpellName, SpellDef);

    UE_LOG(LogTemp, Log, TEXT("Created spell: %s"), *SpellName.ToString());
}

void UGrimoireComponent::RemoveSpell(FName SpellName)
{
    if (ActiveSpells.Contains(SpellName))
    {
        ActiveSpells.Remove(SpellName);
        SpellCooldowns.Remove(SpellName);
        UE_LOG(LogTemp, Log, TEXT("Removed spell: %s"), *SpellName.ToString());
    }
}

void UGrimoireComponent::ExecuteSpell(FName SpellName, AActor* Target, FVector TargetLocation)
{
    // Network handling
    if (GetOwner()->HasAuthority())
    {
        // Server execution
        return ExecuteSpellInternal(SpellName, Target, TargetLocation);
    }
    else
    {
        // Client prediction and server call
        bool bPredictedSuccess = CanCastSpell(SpellName);
        if (bPredictedSuccess)
        {
            // Predict mana consumption
            float ManaCost = CalculateSpellManaCost(SpellName);
            CurrentMana = FMath::Max(0.0f, CurrentMana - ManaCost);
        }
        
        Server_ExecuteSpell(SpellName, Target, TargetLocation);
        return bPredictedSuccess;
    }
}

bool UGrimoireComponent::ExecuteSpellInternal(FName SpellName, AActor* Target, const FVector& TargetLocation)
{
    // Check if spell exists
    if (!ActiveSpells.Contains(SpellName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Spell %s not found"), *SpellName.ToString());
        return false;
    }

    // Check cooldown
    if (SpellCooldowns.Contains(SpellName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Spell %s is on cooldown"), *SpellName.ToString());
        return false;
    }

    // Calculate mana cost and check if we can cast
    float ManaCost = CalculateSpellManaCost(SpellName);
    if (!CanCastSpell(SpellName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot cast spell %s - insufficient mana (%.2f/%.2f)"), 
            *SpellName.ToString(), CurrentMana, ManaCost);
        return false;
    }

    // Create execution context
    USpellExecutionContext* Context = CreateExecutionContext(Target, TargetLocation);
    if (!Context)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create execution context for spell %s"), *SpellName.ToString());
        return false;
    }

    // Execute the spell
    ExecuteSpellInternal(SpellName, Context);

    // Consume mana
    ConsumeMana(Context->ManaCost);

    // Set cooldown
    const FSpellDefinition& SpellDef = ActiveSpells[SpellName];
    if (SpellDef.Cooldown > 0.0f)
    {
        SpellCooldowns.Add(SpellName, SpellDef.Cooldown);
    }

    // Broadcast success
    OnSpellCast.Broadcast(SpellName, true);

    UE_LOG(LogTemp, Log, TEXT("Successfully executed spell %s (Cost: %.2f, Remaining Mana: %.2f)"), 
        *SpellName.ToString(), Context->ManaCost, CurrentMana);

    return true;
}

void UGrimoireComponent::ExecuteSpellInternal(FName SpellName, USpellExecutionContext* Context)
{
    if (!Context)
    {
        return;
    }

    FSpellDefinition* SpellDef = ActiveSpells.Find(SpellName);
    if (!SpellDef || !SpellDef->SpellGraph)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid spell definition for %s"), *SpellName.ToString());
        return;
    }

    // Find the root node (entry point) for execution
    USpellNode* RootNode = FindRootNode(SpellDef->SpellGraph);
    if (!RootNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("No root node found for spell %s"), *SpellName.ToString());
        return;
    }

    // Execute the spell starting from root node
    RootNode->Execute(Context);
}

USpellExecutionContext* UGrimoireComponent::CreateExecutionContext(AActor* Target, const FVector& TargetLocation)
{
    USpellExecutionContext* Context = NewObject<USpellExecutionContext>(this);
    if (!Context)
    {
        return nullptr;
    }

    Context->Initialize(GetOwner(), this);
    
    if (Target)
    {
        Context->SetTarget(Target);
    }
    else if (TargetLocation != FVector::ZeroVector)
    {
        Context->SetTargetLocation(TargetLocation);
    }

    return Context;
}

void UGrimoireComponent::Server_ExecuteSpell_Implementation(FName SpellName, AActor* Target, FVector TargetLocation)
{
    bool bSuccess = ExecuteSpellInternal(SpellName, Target, TargetLocation);
    float ManaCost = bSuccess ? CalculateSpellManaCost(SpellName) : 0.0f;
    
    // Notify client of result
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
    {
        Client_NotifySpellResult(SpellName, bSuccess, ManaCost);
    }
}

void UGrimoireComponent::Client_NotifySpellResult_Implementation(FName SpellName, bool bSuccess, float ManaCost)
{
    // Adjust for any prediction errors
    if (bSuccess)
    {
        // Prediction was correct, no adjustment needed
        UE_LOG(LogTemp, Log, TEXT("Spell %s cast confirmed by server"), *SpellName.ToString());
    }
    else
    {
        // Prediction was wrong, restore mana
        CurrentMana = FMath::Min(MaxMana, CurrentMana + ManaCost);
        UE_LOG(LogTemp, Log, TEXT("Spell %s cast failed, mana restored"), *SpellName.ToString());
    }
    
    OnSpellCast.Broadcast(SpellName, bSuccess);
}

void UGrimoireComponent::Client_PredictExecute(FName SpellName)
{
    // Local execute and deduct
    UHeartGraph** FoundGraph = ActiveSpellGraphs.Find(SpellName);
    if (FoundGraph && *FoundGraph)
    {
        float SpellCost = CalculateSpellManaCost(*FoundGraph);
        if (CanCastSpell(SpellCost))
        {
            if (USpellNode* Root = Cast<USpellNode>((*FoundGraph)->GetRootNode()))
            {
                Root->Execute(this, GetOwner());
            }
            CurrentMana -= SpellCost;  // Predict
        }
    }
}

void UGrimoireComponent::Server_ExecuteSpell_Implementation(FName SpellName)
{
    ExecuteSpell(SpellName);  // Server auth, replicates back if needed
}

float UGrimoireComponent::CalculateSpellManaCost(UHeartGraph* Graph) const
{
    const FSpellDefinition* SpellDef = ActiveSpells.Find(SpellName);
    if (!SpellDef || !SpellDef->SpellGraph)
    {
        return 0.0f;
    }

    float TotalCost = 0.0f;
    
    // Get all nodes in the spell graph
    TArray<UHeartGraphNode*> AllNodes;
    SpellDef->SpellGraph->GetAllNodes(AllNodes);
    
    for (UHeartGraphNode* Node : AllNodes)
    {
        if (USpellNode* SpellNode = Cast<USpellNode>(Node))
        {
            TotalCost += SpellNode->NodeManaCost * SpellNode->GetRarityScaleFactor();
        }
    }

    return TotalCost;
}

bool UGrimoireComponent::CanCastSpell(float SpellManaCost) const
{
    // Check if spell exists
    if (!ActiveSpells.Contains(SpellName))
    {
        return false;
    }

    // Check cooldown
    if (SpellCooldowns.Contains(SpellName))
    {
        return false;
    }

    // Check mana
    float ManaCost = CalculateSpellManaCost(SpellName);
    return CurrentMana >= ManaCost;
}

void UGrimoireComponent::ConsumeMana(float Amount)
{
    CurrentMana = FMath::Max(0.0f, CurrentMana - Amount);
    UE_LOG(LogTemp, Log, TEXT("Consumed %.2f mana, remaining: %.2f"), Amount, CurrentMana);
}

void UGrimoireComponent::CompileAndGrantSpellAbility(FName SpellName, int32 InputID)
{
    UHeartGraph** FoundGraph = ActiveSpell.Find(SpellName);
	if (FoundGraph && *FoundGraph)
    {
        USpellNode* Root = Cast<USpellNode>((*FoundGraph)->GetRootNode());
        if (Root)
        {

        	float Cost;
		    int32 InputID;
        	UGameplayAbility* Ability = Root->CompileToGASAbility(Cost, InputID);
        	if (Ability && AbilitySystem)
        	{
                FGameplayAbilitySpec Spec(Ability, 1, InputID);  // Dynamic
                AbilitySystem->GiveAbility(Spec);
                // Bind to input 
                if (UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent))
                {
                    UInputAction* Action = NewObject<UInputAction>(this);  // Dynamic action per spell
		            FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("Input.Spell.%d"), InputID)));
                    Ability->InputTag = InputTag;  // Custom prop or use BindAbilityToInputTag if extended
                    InputComp->BindAction(Action, ETriggerEvent::Triggered, this, &UGameplayAbility::ActivateAbility, Ability);
                }
	        }
        }
    }
}
USpellNode* UGrimoireComponent::FindRootNode(UHeartGraph* Graph) const
{
    if (!Graph)
    {
        return nullptr;
    }

    TArray<UHeartGraphNode*> AllNodes;
    Graph->GetAllNodes(AllNodes);

    // Look for a node with no incoming execution connections
    for (UHeartGraphNode* Node : AllNodes)
    {
        if (USpellNode* SpellNode = Cast<USpellNode>(Node))
        {
            // Check if this node has incoming execution connections
            TArray<FHeartGraphPinReference> IncomingConnections = Graph->GetConnectedPins(SpellNode->GetNodeGuid(), TEXT("Execute"));
            
            if (IncomingConnections.Num() == 0)
            {
                // This node has no incoming execution, so it's a root
                return SpellNode;
            }
        }
    }

    // If no clear root found, return the first node
    if (AllNodes.Num() > 0)
    {
        return Cast<USpellNode>(AllNodes[0]);
    }

    return nullptr;
}

void UGrimoireComponent::DebugPrintSpellInfo(FName SpellName) const
{
    const FSpellDefinition* SpellDef = ActiveSpells.Find(SpellName);
    if (!SpellDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("Spell %s not found"), *SpellName.ToString());
        return;
    }

    float ManaCost = CalculateSpellManaCost(SpellName);
    bool bCanCast = CanCastSpell(SpellName);
    bool bOnCooldown = SpellCooldowns.Contains(SpellName);

    UE_LOG(LogTemp, Log, TEXT("=== Spell Info: %s ==="), *SpellName.ToString());
    UE_LOG(LogTemp, Log, TEXT("Mana Cost: %.2f"), ManaCost);
    UE_LOG(LogTemp, Log, TEXT("Can Cast: %s"), bCanCast ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("On Cooldown: %s"), bOnCooldown ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Current Mana: %.2f/%.2f"), CurrentMana, MaxMana);

    if (SpellDef->SpellGraph)
    {
        TArray<UHeartGraphNode*> AllNodes;
        SpellDef->SpellGraph->GetAllNodes(AllNodes);
        UE_LOG(LogTemp, Log, TEXT("Node Count: %d"), AllNodes.Num());
    }
}

UFUNCTION()
void ActivateSpellAbility(UGameplayAbility* Ability)
{
    AbilitySystem->TryActivateAbility(Ability->GetCurrentAbilitySpecHandle());
}