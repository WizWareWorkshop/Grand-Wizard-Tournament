#include "Spells/ConditionNode.h"
#include "SpellExecutionContext.h"
#include "Components/GrimoireComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Model/HeartGraph.h"
#include "Math/UnrealMathUtility.h"

UConditionNode::UConditionNode()
{
    NodeName = TEXT("Condition Node");
    NodeType = ESpellNodeType::Condition;
    NodeDescription = FText::FromString(TEXT("Creates branching logic in spells"));
    NodeIcon = FText::FromString(TEXT("❓"));
    NodeColor = FLinearColor::Yellow;
    NodeCategory = FText::FromString(TEXT("Logic"));
    
    ConditionType = EConditionType::IfThen;
    ComparisonOperator = EComparisonOperator::Greater;
    ComparisonValue = 0.5f;
    RandomChance = 0.5f;
    VariableName = TEXT("Health");
    NodeManaCost = 5.0f;
}

TArray<FHeartGraphPinDesc> UConditionNode::GetInputPinDescs() const
{
    TArray<FHeartGraphPinDesc> Pins = Super::GetInputPinDescs();
    
    // Condition-specific input pins
    Pins.Add(CreateDataPin(TEXT("ConditionValue"), EHeartPinDirection::Input, TEXT("Float")));
    Pins.Add(CreateDataPin(TEXT("CompareValue"), EHeartPinDirection::Input, TEXT("Float")));
    Pins.Add(CreateDataPin(TEXT("BoolCondition"), EHeartPinDirection::Input, TEXT("Bool")));
    
    return Pins;
}

TArray<FHeartGraphPinDesc> UConditionNode::GetOutputPinDescs() const
{
    TArray<FHeartGraphPinDesc> Pins = Super::GetOutputPinDescs();
    
    // Remove the default "Then" pin since we have specific True/False pins
    Pins.RemoveAll([](const FHeartGraphPinDesc& Pin) { return Pin.Name == TEXT("Then"); });
    
    // Condition-specific output pins
    Pins.Add(CreateExecutionPin(TEXT("True"), EHeartPinDirection::Output));
    Pins.Add(CreateExecutionPin(TEXT("False"), EHeartPinDirection::Output));
    Pins.Add(CreateDataPin(TEXT("Result"), EHeartPinDirection::Output, TEXT("Bool")));
    
    return Pins;
}

void UConditionNode::OnExecute(USpellExecutionContext* Context)
{
    Super::OnExecute(Context);
    
    if (!Context)
    {
        UE_LOG(LogTemp, Error, TEXT("ConditionNode::OnExecute - Context is null"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("ConditionNode executing: %s, Type: %s"), 
        *NodeName, 
        *UEnum::GetValueAsString(ConditionType));
    
    // Cache connected nodes for efficiency
    CacheConnectedNodes();
    
    // Evaluate the condition
    bool bConditionResult = EvaluateCondition(Context);
    
    // Store result in context
    Context->SetVariable(TEXT("ConditionResult"), FGWTVariableValue::FromBool(bConditionResult));
    Context->SetVariable(TEXT("LastConditionNode"), FGWTVariableValue::FromString(NodeName));
    
    UE_LOG(LogTemp, Log, TEXT("Condition %s evaluated to: %s"), 
        *NodeName, bConditionResult ? TEXT("TRUE") : TEXT("FALSE"));
    
    // Apply rarity effects before branching
    ApplyRarityEffects(Context, bConditionResult);
    
    // Execute appropriate branch
    if (bConditionResult)
    {
        ExecuteTrueBranch(Context);
    }
    else if (ConditionType == EConditionType::IfThenElse)
    {
        ExecuteFalseBranch(Context);
    }
}

bool UConditionNode::EvaluateCondition(USpellExecutionContext* Context)
{
    switch (ConditionType)
    {
        case EConditionType::IfThen:
        case EConditionType::IfThenElse:
            return EvaluateIfThen(Context);
            
        case EConditionType::Compare:
            return EvaluateComparison(Context);
            
        case EConditionType::HealthCheck:
            return EvaluateHealthCheck(Context);
            
        case EConditionType::DistanceCheck:
            return EvaluateDistanceCheck(Context);
            
        case EConditionType::RandomChance:
            return EvaluateRandomChance(Context);
            
        case EConditionType::HasStatus:
            return EvaluateHasStatus(Context);
            
        case EConditionType::TimeBased:
            return EvaluateTimeBased(Context);
            
        default:
            return false;
    }
}

bool UConditionNode::EvaluateIfThen(USpellExecutionContext* Context)
{
    // Check for boolean input from connected nodes
    if (Context->HasVariable(TEXT("BoolCondition")))
    {
        FGWTVariableValue BoolVar = Context->GetVariable(TEXT("BoolCondition"));
        if (BoolVar.Type == EGWTVariableType::Bool)
        {
            return BoolVar.BoolValue;
        }
    }
    
    // Check for named variable
    if (Context->HasVariable(VariableName))
    {
        FGWTVariableValue Var = Context->GetVariable(VariableName);
        switch (Var.Type)
        {
            case EGWTVariableType::Bool:
                return Var.BoolValue;
            case EGWTVariableType::Float:
                return Var.FloatValue > 0.0f;
            case EGWTVariableType::Int:
                return Var.IntValue > 0;
            default:
                return false;
        }
    }
    
    // Default: compare spell power to threshold
    return Context->SpellPower > ComparisonValue;
}

bool UConditionNode::EvaluateComparison(USpellExecutionContext* Context)
{
    float ValueA = GetVariableValue(Context, TEXT("ConditionValue"));
    float ValueB = ComparisonValue;
    
    // Check for comparison value override
    if (Context->HasVariable(TEXT("CompareValue")))
    {
        FGWTVariableValue CompareVar = Context->GetVariable(TEXT("CompareValue"));
        if (CompareVar.Type == EGWTVariableType::Float)
        {
            ValueB = CompareVar.FloatValue;
        }
        else if (CompareVar.Type == EGWTVariableType::Int)
        {
            ValueB = static_cast<float>(CompareVar.IntValue);
        }
    }
    
    return CompareValues(ValueA, ValueB, ComparisonOperator);
}

bool UConditionNode::EvaluateHealthCheck(USpellExecutionContext* Context)
{
    AActor* Target = Context->Target ? Context->Target : Context->Caster;
    if (!Target)
    {
        return false;
    }
    
    // In a real implementation, you'd have a health component
    // For now, simulate with a random health percentage
    float HealthPercentage = FMath::RandRange(0.0f, 1.0f);
    
    // Store health info in context
    Context->SetVariable(TEXT("TargetHealth"), FGWTVariableValue::FromFloat(HealthPercentage));
    
    return CompareValues(HealthPercentage, ComparisonValue, ComparisonOperator);
}

bool UConditionNode::EvaluateDistanceCheck(USpellExecutionContext* Context)
{
    if (!Context->Caster || !Context->Target)
    {
        return false;
    }
    
    float Distance = FVector::Dist(Context->Caster->GetActorLocation(), Context->Target->GetActorLocation());
    Context->SetVariable(TEXT("Distance"), FGWTVariableValue::FromFloat(Distance));
    
    return CompareValues(Distance, ComparisonValue, ComparisonOperator);
}

bool UConditionNode::EvaluateRandomChance(USpellExecutionContext* Context)
{
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    float AdjustedChance = RandomChance;
    
    // Rarity affects random chance
    switch (NodeRarity)
    {
        case EItemRarity::Uncommon:
            AdjustedChance *= 1.1f; // 10% better odds
            break;
        case EItemRarity::Rare:
            AdjustedChance *= 1.25f; // 25% better odds
            break;
        case EItemRarity::Epic:
            AdjustedChance *= 1.5f; // 50% better odds
            break;
        case EItemRarity::Legendary:
            AdjustedChance = 1.0f; // Always succeeds
            break;
        default:
            break;
    }
    
    AdjustedChance = FMath::Clamp(AdjustedChance, 0.0f, 1.0f);
    
    Context->SetVariable(TEXT("RandomValue"), FGWTVariableValue::FromFloat(RandomValue));
    Context->SetVariable(TEXT("ChanceThreshold"), FGWTVariableValue::FromFloat(AdjustedChance));
    
    return RandomValue <= AdjustedChance;
}

bool UConditionNode::EvaluateHasStatus(USpellExecutionContext* Context)
{
    AActor* Target = Context->Target ? Context->Target : Context->Caster;
    if (!Target)
    {
        return false;
    }
    
    // In a real implementation, you'd check for actual status effects
    // For now, simulate random status check
    bool bHasStatus = FMath::RandBool();
    
    Context->SetVariable(TEXT("HasStatus"), FGWTVariableValue::FromBool(bHasStatus));
    
    return bHasStatus;
}

bool UConditionNode::EvaluateTimeBased(USpellExecutionContext* Context)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    float CurrentTime = World->GetTimeSeconds();
    float TimeSinceStart = CurrentTime - Context->ExecutionTime;
    
    Context->SetVariable(TEXT("TimeSinceStart"), FGWTVariableValue::FromFloat(TimeSinceStart));
    Context->SetVariable(TEXT("CurrentTime"), FGWTVariableValue::FromFloat(CurrentTime));
    
    return CompareValues(TimeSinceStart, ComparisonValue, ComparisonOperator);
}

float UConditionNode::GetVariableValue(USpellExecutionContext* Context, FName VarName)
{
    if (!Context->HasVariable(VarName))
    {
        return 0.0f;
    }
    
    FGWTVariableValue Var = Context->GetVariable(VarName);
    switch (Var.Type)
    {
        case EGWTVariableType::Float:
            return Var.FloatValue;
        case EGWTVariableType::Int:
            return static_cast<float>(Var.IntValue);
        case EGWTVariableType::Bool:
            return Var.BoolValue ? 1.0f : 0.0f;
        default:
            return 0.0f;
    }
}

bool UConditionNode::CompareValues(float A, float B, EComparisonOperator Operator)
{
    switch (Operator)
    {
        case EComparisonOperator::Equal:
            return FMath::IsNearlyEqual(A, B, 0.001f);
        case EComparisonOperator::NotEqual:
            return !FMath::IsNearlyEqual(A, B, 0.001f);
        case EComparisonOperator::Greater:
            return A > B;
        case EComparisonOperator::GreaterEqual:
            return A >= B;
        case EComparisonOperator::Less:
            return A < B;
        case EComparisonOperator::LessEqual:
            return A <= B;
        default:
            return false;
    }
}

void UConditionNode::ExecuteTrueBranch(USpellExecutionContext* Context)
{
    if (TrueBranchNodes.Num() == 0)
    {
        // Try to get connected nodes from graph
        if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
        {
            TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), TEXT("True"));
            
            for (const FHeartGraphPinReference& Connection : Connections)
            {
                if (UHeartGraphNode* Node = Graph->GetNode(Connection.NodeGuid))
                {
                    if (USpellNode* SpellNode = Cast<USpellNode>(Node))
                    {
                        SpellNode->Execute(Context);
                    }
                }
            }
        }
    }
    else
    {
        // Use cached nodes
        for (USpellNode* Node : TrueBranchNodes)
        {
            if (Node && IsValid(Node))
            {
                Node->Execute(Context);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Executed TRUE branch for condition %s"), *NodeName);
}

void UConditionNode::ExecuteFalseBranch(USpellExecutionContext* Context)
{
    if (FalseBranchNodes.Num() == 0)
    {
        // Try to get connected nodes from graph
        if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
        {
            TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), TEXT("False"));
            
            for (const FHeartGraphPinReference& Connection : Connections)
            {
                if (UHeartGraphNode* Node = Graph->GetNode(Connection.NodeGuid))
                {
                    if (USpellNode* SpellNode = Cast<USpellNode>(Node))
                    {
                        SpellNode->Execute(Context);
                    }
                }
            }
        }
    }
    else
    {
        // Use cached nodes
        for (USpellNode* Node : FalseBranchNodes)
        {
            if (Node && IsValid(Node))
            {
                Node->Execute(Context);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Executed FALSE branch for condition %s"), *NodeName);
}

void UConditionNode::ApplyRarityEffects(USpellExecutionContext* Context, bool bConditionResult)
{
    switch (NodeRarity)
    {
        case EItemRarity::Uncommon:
            // Store additional condition info
            Context->SetVariable(TEXT("ConditionConfidence"), FGWTVariableValue::FromFloat(0.8f));
            break;
            
        case EItemRarity::Rare:
            // Can store condition history
            Context->SetVariable(TEXT("PreviousCondition"), Context->GetVariable(TEXT("LastConditionResult")));
            Context->SetVariable(TEXT("LastConditionResult"), FGWTVariableValue::FromBool(bConditionResult));
            break;
            
        case EItemRarity::Epic:
            // Complex condition support - can evaluate multiple conditions
            if (Context->HasVariable(TEXT("SecondaryCondition")))
            {
                bool bSecondaryResult = Context->GetVariable(TEXT("SecondaryCondition")).BoolValue;
                Context->SetVariable(TEXT("ComplexCondition"), FGWTVariableValue::FromBool(bConditionResult && bSecondaryResult));
            }
            break;
            
        case EItemRarity::Legendary:
            // Quantum conditioning - executes both branches with different contexts
            if (bConditionResult)
            {
                // Create alternative context for false branch
                USpellExecutionContext* AltContext = Context->CreateChildContext();
                AltContext->ModifySpellPower(0.5f); // Reduced power for quantum branch
                ExecuteFalseBranch(AltContext);
            }
            break;
    }
}

void UConditionNode::CacheConnectedNodes()
{
    TrueBranchNodes.Empty();
    FalseBranchNodes.Empty();
    
    if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
    {
        // Cache True branch connections
        TArray<FHeartGraphPinReference> TrueConnections = Graph->GetConnectedPins(GetNodeGuid(), TEXT("True"));
        for (const FHeartGraphPinReference& Connection : TrueConnections)
        {
            if (UHeartGraphNode* Node = Graph->GetNode(Connection.NodeGuid))
            {
                if (USpellNode* SpellNode = Cast<USpellNode>(Node))
                {
                    TrueBranchNodes.Add(SpellNode);
                }
            }
        }
        
        // Cache False branch connections
        TArray<FHeartGraphPinReference> FalseConnections = Graph->GetConnectedPins(GetNodeGuid(), TEXT("False"));
        for (const FHeartGraphPinReference& Connection : FalseConnections)
        {
            if (UHeartGraphNode* Node = Graph->GetNode(Connection.NodeGuid))
            {
                if (USpellNode* SpellNode = Cast<USpellNode>(Node))
                {
                    FalseBranchNodes.Add(SpellNode);
                }
            }
        }
    }
}

float UConditionNode::GetBasePower() const
{
    return 0.0f; // Conditions don't provide power directly
}