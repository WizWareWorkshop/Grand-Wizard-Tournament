#include "Components/GrimoireComponent.h"
#include "Model/HeartGraph.h"
#include "Spells/SpellNode.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"

UGrimoireComponent::UGrimoireComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentMana = 100.0f;
    MaxMana = 100.0f;
	ManaRegenRate = 5.0f;  
    BaseDamage = 10.0f;
    SetIsReplicatedByDefault(true);
}

void UGrimoireComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGrimoireComponent, AvailableNodes);
    DOREPLIFETIME(UGrimoireComponent, ActiveSpells);
    DOREPLIFETIME(UGrimoireComponent, CurrentMana);
}

void UGrimoireComponent::BeginPlay()
{
    Super::BeginPlay();
    if (AActor* Owner = GetOwner())
    {
        AbilitySystem = Owner->FindComponentByClass<UAbilitySystemComponent>();
        if (!AbilitySystem)
        {
            AbilitySystem = NewObject<UAbilitySystemComponent>(Owner);
            AbilitySystem->RegisterComponent();
        }
	    // Enhanced Input setup 
        if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
            {
                Subsystem->AddMappingContext(InputContext, 0);
        }
    }
}

void UGrimoireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CurrentMana = FMath::Min(MaxMana, CurrentMana + ManaRegenRate * DeltaTime);
}

void UGrimoireComponent::OnRep_CurrentMana()
{
    OnManaChanged.Broadcast(CurrentMana);
    UE_LOG(LogTemp, Log, TEXT("Mana replicated: %f"), CurrentMana);
}

void UGrimoireComponent::AddSpellNode(TSubclassOf<USpellNode> SpellNodeClass)
{
    if (!SpellNodeClass) return;

    USpellNode* NewNode = NewObject<USpellNode>(this, SpellNodeClass);
    if (NewNode)
    {
        AvailableNodes.AddUnique(NewNode);
    }
}

void UGrimoireComponent::RemoveSpellNode(USpellNode* SpellNode)
{
    if (!SpellNode) return;

    AvailableNodes.Remove(SpellNode);
   
    for (auto It = ActiveSpells.CreateIterator(); It; ++It)
    {
        if (It.Value() == SpellNode)
        {
            It.RemoveCurrent();
            break;
        }
    }
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

void UGrimoireComponent::ExecuteSpell(FName SpellName)
{
	if (GetNetMode() == NM_Client)
    {
        Client_PredictExecute(SpellName);
        Server_ExecuteSpell(SpellName);
    } else {
        // Server direct
        UHeartGraph** FoundGraph = ActiveSpells.Find(SpellName);
        if (!FoundGraph || !*FoundGraph) return;

        float SpellCost = CalculateSpellManaCost(*FoundGraph);
        if (CanCastSpell(SpellCost))
        {
            if (USpellNode* Root = Cast<USpellNode>((*FoundGraph)->GetRootNode()))  // Assume root is trigger or magic
            {
                Root->Execute(this, GetOwner());
            }
            ConsumeMana(SpellCost);
        }
    }
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
    float Total = 0.0f;
    TArray<UHeartGraphNode*> AllNodes;
    Graph->GetAllNodes(AllNodes);
    for (UHeartGraphNode* HNode : AllNodes)
    {
        if (USpellNode* SNode = Cast<USpellNode>(HNode))
        {
            Total += SNode->NodeManaCost;
        }
    }
    return Total;
}

bool UGrimoireComponent::CanCastSpell(float SpellManaCost) const
{
    return CurrentMana >= SpellManaCost;
}

void UGrimoireComponent::ConsumeMana(float Amount)
{
    CurrentMana = FMath::Max(0.0f, CurrentMana - Amount);
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
    TArray<UHeartGraphNode*> AllNodes;
    Graph->GetAllNodes(AllNodes);
    for (UHeartGraphNode* HNode : AllNodes)
    {
        if (USpellNode* SNode = Cast<USpellNode>(HNode))
        {
            // Assume root is one with no incoming ExecIn
            if (Graph->GetConnectedPins(SNode->GetPinReference(TEXT("ExecIn"))).Num() == 0)
            {
                return SNode;
            }
        }
    }
    return nullptr;
}

UFUNCTION()
void ActivateSpellAbility(UGameplayAbility* Ability)
{
    AbilitySystem->TryActivateAbility(Ability->GetCurrentAbilitySpecHandle());
}