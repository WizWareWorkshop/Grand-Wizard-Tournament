#include "Spells/SpellNode.h"
#include "Components/GrimoireComponent.h"
#include "Model/HeartGraph.h"
#include "Model/HeartGraphNode.h"
#include "BloodProperty.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"
#include "Misc/AssertionMacros.h"

USpellNode::USpellNode()
{
    NodeID = FGuid::NewGuid();
    Position = FVector2D::ZeroVector;
    NodeRarity = EItemRarity::Common;
    NodeName = TEXT("New Node");
    NodeDescription = FText::FromString(TEXT("New Node Description"));
    NodeIcon = FText::FromString(TEXT("New Node Icon"));
    NodeColor = FLinearColor::White;
    NodeType = ESpellNodeType::Magic;
    NodeCategory = FText::FromString(TEXT("Spells"));
    NodeCost = 0;
    NodeManaCost = 0;
}

void USpellNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USpellNode, NodeRarity);
}

void USpellNode::PostInitProperties()
{
    Super::PostInitProperties();
    InitializeDefaultPins();
}

void USpellNode::InitializeDefaultPins()
{
    // Use Heart's Blood for pins (example: Execution in/out)
    CreateInputPin(FHeartPinDesc(TEXT("ExecIn"), EHeartPinDirection::Input, FHeartBloodType::Execution));
    CreateOutputPin(FHeartPinDesc(TEXT("ExecOut"), EHeartPinDirection::Output, FHeartBloodType::Execution));
    // Subclasses add more (e.g., data pins)
    FHeartGraphPinDesc ExecInDesc;
    ExecInDesc.Name = TEXT("ExecIn");
    ExecInDesc.Direction = EHeartPinDirection::Input;
    ExecInDesc.BloodType = FHeartGraphBloodType::Execution;
    InputPins.Add(CreatePin(ExecInDesc));

    FHeartGraphPinDesc ExecOutDesc;
    ExecOutDesc.Name = TEXT("ExecOut");
    ExecOutDesc.Direction = EHeartPinDirection::Output;
    ExecOutDesc.BloodType = FHeartBloodType::Execution;
    OutputPins.Add(CreatePin(ExecOutDesc));
}

TArray<FHeartGraphPinDesc> USpellNode::GetInputPins() const
{
    TArray<FHeartGraphPinDesc> Pins;
    Pins.Add(FHeartGraphPinDesc(TEXT("ExecIn"), EHeartPinDirection::Input, FHeartBloodType::Execution));
    // Base data pin
    Pins.Add(FHeartGraphPinDesc(TEXT("DataIn"), EHeartPinDirection::Input, FHeartBloodType::Struct));
    return Pins;
}

TArray<FHeartPinDesc> USpellNode::GetOutputPins() const
{
    TArray<FHeartPinDesc> Pins;
    Pins.Add(FHeartPinDesc(TEXT("ExecOut"), EHeartPinDirection::Output, FHeartBloodType::Execution));
    // Base data pin
    Pins.Add(FHeartPinDesc(TEXT("DataOut"), EHeartPinDirection::Output, FHeartBloodType::Struct));
    return Pins;
}

TArray<FHeartGraphPinDesc> USpellNode::GetPins(EHeartPinDirection Direction) const
{
    TArray<FHeartGraphPinDesc> Pins;
    if (Direction == EHeartPinDirection::Input)
    {
        for (const FHeartGraphPinReference& Pin : InputPins)
        {
            Pins.Add(Pin.Desc); 
        }
    } else if (Direction == EHeartPinDirection::Output)
    {
        for (const FHeartGraphPinReference& Pin : OutputPins)
        {
            Pins.Add(Pin.Desc);
        }
    }
    return Pins;
}

void USpellNode::Execute(UGrimoireComponent* Grimoire, AActor* ContextActor)
{
    if (!ContextActor || !Grimoire) return;

    OnExecute(Grimoire, ContextActor);

    // Execute connected nodes through HeartGraph connections
    TArray<USpellNode*> ConnectedNodes = GetConnectedSpellNodes();
    for (USpellNode* ConnectedNode : ConnectedNodes)
    {
        if (ConnectedNode && IsValid(ConnectedNode))
        {
            ConnectedNode->Execute(Grimoire, ContextActor);
        }
    }

    OnExecutionComplete(ContextActor);
}

TArray<USpellNode*> USpellNode::GetConnectedSpellNodes() const
{
    TArray<USpellNode*> ConnectedNodes;
    if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
    {
        FHeartGraphPinReference ExecOutPin = GetPinReference(TEXT("ExecOut"));
        TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), TEXT("ExecOut"));
        for (const FHeartGraphPinReference& Ref : Connections)
        {
            UHeartGraphNode* Node = Graph->GetNode(Conn.OtherNode);
            if (UHeartGraphNode* Node = Graph->GetNode(Ref.NodeGuid))
            {
                if (USpellNode* SpellNode = Cast<USpellNode>(Node))
                {
                    ConnectedNodes.Add(SpellNode);
                }
            }
        }
    }
    return ConnectedNodes;
}

void USpellNode::OnExecute(UGrimoireComponent* Grimoire, AActor* ContextActor)
{
    // Base implementation - override in derived classes
    UE_LOG(LogTemp, Log, TEXT("Executing node: %s"), *NodeName);
}

void USpellNode::OnExecutionComplete(AActor* ContextActor)
{
    UE_LOG(LogTemp, Log, TEXT("Node execution complete: %s"), *NodeName);
}

float USpellNode::GetBasePower() const
{
    return 1.0f * GetRarityScaleFactor();
}

float USpellNode::GetRarityScaleFactor() const
{
    switch (NodeRarity)
    {
        case EItemRarity::Common: return 1.0f;
        case EItemRarity::Uncommon: return 1.25f;
        case EItemRarity::Rare: return 1.5f;
        case EItemRarity::Epic: return 2.0f;
        case EItemRarity::Legendary: return 2.5f;
        default: return 1.0f;
    }
}

int32 USpellNode::GetMaxInputConnections() const
{
    switch (NodeRarity)
    {
        case EItemRarity::Common: return 1;
        case EItemRarity::Uncommon: return 2;
        case EItemRarity::Rare: return 3;
        case EItemRarity::Epic: return 4;
        case EItemRarity::Legendary: return 5;
        default: return 1;
    }
}

int32 USpellNode::GetMaxOutputConnections() const
{
    return GetMaxInputConnections();
}

UGameplayAbility* USpellNode::CompileToGASAbility(float& OutSpellManaCost, int32& OutInputID) const
{
	OutSpellManaCost = NodeManaCost;
	OutInputID = static_cast<int32>(EGWTAbilityInputID::None);
    // Dynamic GAS creation
    UGameplayAbility* Ability = NewObject<UGameplayAbility>();
    // Map node logic to ability tasks (e.g., add UAbilityTask_PlayMontageAndWait for animations)
    // Example stub: Set cost based on mana
    Ability->AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Spell.Basic")));
    TArray<USpellNode*> Connected = GetConnectedSpellNodes();
    for (USpellNode* Node : Connected)
    {
        float NodeCost;
	    int32 NodeID;
        Node->CompileToGASAbility(NodeCost, NodeID);  // Recursive add
        OutSpellManaCost += NodeCost;
	    if (NodeID != static_cast<int32>(EAbilityInputID::None)) OutInputID = NodeID;
    }
    return Ability;
}