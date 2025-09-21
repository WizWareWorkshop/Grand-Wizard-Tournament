#include "Spells/FlowNode.h"
#include "SpellExecutionContext.h"
#include "Components/GrimoireComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Model/HeartGraph.h"

UFlowNode::UFlowNode()
{
    NodeName = TEXT("Flow Node");
    NodeType = ESpellNodeType::Flow;
    NodeDescription = FText::FromString(TEXT("Controls execution flow and timing"));
    NodeIcon = FText::FromString(TEXT("🔄"));
    NodeColor = FLinearColor::Cyan;
    NodeCategory = FText::FromString(TEXT("Flow Control"));
    
    FlowType = EFlowNodeType::Sequence;
    MaxIterations = 5;
    DelayTime = 1.0f;
    IterationDelay = 0.1f;
    bBreakOnCondition = false;
    BreakConditionVariable = TEXT("ShouldBreak");
    NodeManaCost = 8.0f;
    
    CachedContext = nullptr;
    RecursionDepth = 0;
    CompletedParallelNodes = 0;
}

TArray<FHeartGraphPinDesc> UFlowNode::GetInputPinDescs() const
{
    TArray<FHeartGraphPinDesc> Pins = Super::GetInputPinDescs();
    
    // Flow-specific input pins
    Pins.Add(CreateDataPin(TEXT("IterationCount"), EHeartPinDirection::Input, TEXT("Int")));
    Pins.Add(CreateDataPin(TEXT("Condition"), EHeartPinDirection::Input, TEXT("Bool")));
    Pins.Add(CreateDataPin(TEXT("DelayTime"), EHeartPinDirection::Input, TEXT("Float")));
    
    return Pins;
}

TArray<FHeartGraphPinDesc> UFlowNode::GetOutputPinDescs() const
{
    TArray<FHeartGraphPinDesc> Pins = Super::GetOutputPinDescs();
    
    // Remove default "Then" since we have specific flow outputs
    Pins.RemoveAll([](const FHeartGraphPinDesc& Pin) { return Pin.Name == TEXT("Then"); });
    
    // Flow-specific output pins based on type
    switch (FlowType)
    {
        case EFlowNodeType::Loop:
        case EFlowNodeType::WhileLoop:
        case EFlowNodeType::ForLoop:
            Pins.Add(CreateExecutionPin(TEXT("LoopBody"), EHeartPinDirection::Output));
            Pins.Add(CreateExecutionPin(TEXT("OnComplete"), EHeartPinDirection::Output));
            Pins.Add(CreateDataPin(TEXT("CurrentIndex"), EHeartPinDirection::Output, TEXT("Int")));
            break;
            
        case EFlowNodeType::Parallel:
            Pins.Add(CreateExecutionPin(TEXT("Branch1"), EHeartPinDirection::Output));
            Pins.Add(CreateExecutionPin(TEXT("Branch2"), EHeartPinDirection::Output));
            Pins.Add(CreateExecutionPin(TEXT("Branch3"), EHeartPinDirection::Output));
            Pins.Add(CreateExecutionPin(TEXT("OnAllComplete"), EHeartPinDirection::Output));
            break;
            
        case EFlowNodeType::Branch:
            Pins.Add(CreateExecutionPin(TEXT("True"), EHeartPinDirection::Output));
            Pins.Add(CreateExecutionPin(TEXT("False"), EHeartPinDirection::Output));
            break;
            
        case EFlowNodeType::Gate:
            Pins.Add(CreateExecutionPin(TEXT("Open"), EHeartPinDirection::Output));
            Pins.Add(CreateExecutionPin(TEXT("Closed"), EHeartPinDirection::Output));
            break;
            
        default:
            Pins.Add(CreateExecutionPin(TEXT("Next"), EHeartPinDirection::Output));
            break;
    }
    
    return Pins;
}

void UFlowNode::OnExecute(USpellExecutionContext* Context)
{
    Super::OnExecute(Context);
    
    if (!Context)
    {
        UE_LOG(LogTemp, Error, TEXT("FlowNode::OnExecute - Context is null"));
        return;
    }
    
    // Check recursion limit
    if (!CheckRecursionLimit(Context))
    {
        UE_LOG(LogTemp, Error, TEXT("FlowNode recursion limit exceeded for %s"), *NodeName);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("FlowNode executing: %s, Type: %s"), 
        *NodeName, 
        *UEnum::GetValueAsString(FlowType));
    
    // Cache context for timer callbacks
    CachedContext = Context;
    
    // Apply rarity effects before execution
    ApplyRarityEffects(Context);
    
    // Execute based on flow type
    switch (FlowType)
    {
        case EFlowNodeType::Sequence:
            ExecuteSequenceInternal(Context);
            break;
            
        case EFlowNodeType::Loop:
            ExecuteLoopInternal(Context);
            break;
            
        case EFlowNodeType::WhileLoop:
            ExecuteWhileLoop(Context);
            break;
            
        case EFlowNodeType::ForLoop:
            ExecuteForLoop(Context);
            break;
            
        case EFlowNodeType::Delay:
            ExecuteDelayInternal(Context);
            break;
            
        case EFlowNodeType::Parallel:
            ExecuteParallelInternal(Context);
            break;
            
        case EFlowNodeType::Branch:
            ExecuteBranch(Context);
            break;
            
        case EFlowNodeType::Gate:
            ExecuteGate(Context);
            break;
    }
}

void UFlowNode::ExecuteSequenceInternal(USpellExecutionContext* Context)
{
    // Sequential execution of connected nodes
    TArray<USpellNode*> SequenceNodes = GetConnectedOutputNodes();
    
    for (int32 i = 0; i < SequenceNodes.Num(); i++)
    {
        USpellNode* Node = SequenceNodes[i];
        if (Node && IsValid(Node))
        {
            // Set sequence info in context
            Context->SetVariable(TEXT("SequenceIndex"), FGWTVariableValue::FromInt(i));
            Context->SetVariable(TEXT("SequenceTotal"), FGWTVariableValue::FromInt(SequenceNodes.Num()));
            
            Node->Execute(Context);
            
            // Check for break condition
            if (bBreakOnCondition && EvaluateBreakCondition(Context))
            {
                UE_LOG(LogTemp, Log, TEXT("Sequence break condition met at index %d"), i);
                break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Sequence completed for %s"), *NodeName);
}

void UFlowNode::ExecuteLoopInternal(USpellExecutionContext* Context)
{
    InitializeLoopState(Context);
    
    TArray<USpellNode*> LoopBodyNodes = GetLoopBodyNodes();
    
    while (ShouldContinueLoop(Context))
    {
        // Set loop variables in context
        Context->SetVariable(TEXT("LoopIndex"), FGWTVariableValue::FromInt(CurrentLoopState.CurrentIteration));
        Context->SetVariable(TEXT("LoopTotal"), FGWTVariableValue::FromInt(CurrentLoopState.MaxIterations));
        
        // Execute loop body
        for (USpellNode* Node : LoopBodyNodes)
        {
            if (Node && IsValid(Node))
            {
                Node->Execute(Context);
            }
        }
        
        UpdateLoopState(Context);
        
        // Add iteration delay if specified
        if (IterationDelay > 0.0f)
        {
            // For now, just log the delay (in a real implementation, you'd use timers)
            UE_LOG(LogTemp, Log, TEXT("Loop iteration delay: %.2f seconds"), IterationDelay);
        }
    }
    
    // Execute completion nodes
    if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
    {
        TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), TEXT("OnComplete"));
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
    
    UE_LOG(LogTemp, Log, TEXT("Loop completed for %s (%d iterations)"), 
        *NodeName, CurrentLoopState.CurrentIteration);
}

void UFlowNode::ExecuteWhileLoop(USpellExecutionContext* Context)
{
    InitializeLoopState(Context);
    
    TArray<USpellNode*> LoopBodyNodes = GetLoopBodyNodes();
    
    while (CurrentLoopState.bShouldContinue && 
           CurrentLoopState.CurrentIteration < CurrentLoopState.MaxIterations)
    {
        // Check while condition
        bool bCondition = true;
        if (Context->HasVariable(TEXT("Condition")))
        {
            bCondition = Context->GetVariable(TEXT("Condition")).BoolValue;
        }
        
        if (!bCondition)
        {
            break;
        }
        
        // Execute loop body
        Context->SetVariable(TEXT("WhileIndex"), FGWTVariableValue::FromInt(CurrentLoopState.CurrentIteration));
        
        for (USpellNode* Node : LoopBodyNodes)
        {
            if (Node && IsValid(Node))
            {
                Node->Execute(Context);
            }
        }
        
        CurrentLoopState.CurrentIteration++;
        
        // Safety check for infinite loops
        if (CurrentLoopState.CurrentIteration > 1000)
        {
            UE_LOG(LogTemp, Warning, TEXT("While loop safety limit reached for %s"), *NodeName);
            break;
        }
    }
}

void UFlowNode::ExecuteForLoop(USpellExecutionContext* Context)
{
    int32 LoopCount = MaxIterations;
    
    // Override with input if available
    if (Context->HasVariable(TEXT("IterationCount")))
    {
        FGWTVariableValue CountVar = Context->GetVariable(TEXT("IterationCount"));
        if (CountVar.Type == EGWTVariableType::Int)
        {
            LoopCount = CountVar.IntValue;
        }
    }
    
    // Apply rarity scaling
    LoopCount = FMath::Min(LoopCount * GetRarityScaleFactor(), GetMaxIterationsByRarity());
    
    TArray<USpellNode*> LoopBodyNodes = GetLoopBodyNodes();
    
    for (int32 i = 0; i < LoopCount; i++)
    {
        Context->SetVariable(TEXT("ForIndex"), FGWTVariableValue::FromInt(i));
        Context->SetVariable(TEXT("ForTotal"), FGWTVariableValue::FromInt(LoopCount));
        
        for (USpellNode* Node : LoopBodyNodes)
        {
            if (Node && IsValid(Node))
            {
                Node->Execute(Context);
            }
        }
        
        // Check break condition
        if (bBreakOnCondition && EvaluateBreakCondition(Context))
        {
            UE_LOG(LogTemp, Log, TEXT("For loop break condition met at iteration %d"), i);
            break;
        }
    }
}

void UFlowNode::ExecuteDelayInternal(USpellExecutionContext* Context)
{
    float Delay = DelayTime;
    
    // Override with input if available
    if (Context->HasVariable(TEXT("DelayTime")))
    {
        FGWTVariableValue DelayVar = Context->GetVariable(TEXT("DelayTime"));
        if (DelayVar.Type == EGWTVariableType::Float)
        {
            Delay = DelayVar.FloatValue;
        }
    }
    
    // Apply rarity effects to delay
    Delay /= GetRarityScaleFactor(); // Higher rarity = shorter delay
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            DelayTimerHandle,
            this,
            &UFlowNode::HandleDelayComplete,
            Delay,
            false
        );
        
        UE_LOG(LogTemp, Log, TEXT("Started delay of %.2f seconds for %s"), Delay, *NodeName);
    }
}

void UFlowNode::ExecuteParallelInternal(USpellExecutionContext* Context)
{
    ActiveParallelNodes = GetParallelNodes();
    CompletedParallelNodes = 0;
    
    // Execute all parallel branches simultaneously
    for (USpellNode* Node : ActiveParallelNodes)
    {
        if (Node && IsValid(Node))
        {
            // Create child context for each parallel branch
            USpellExecutionContext* ChildContext = Context->CreateChildContext();
            Node->Execute(ChildContext);
            
            // In a real implementation, you'd track completion asynchronously
            CompletedParallelNodes++;
        }
    }
    
    // For now, assume all complete immediately
    // In a real system, you'd wait for async completion
    if (CompletedParallelNodes >= ActiveParallelNodes.Num())
    {
        // Execute completion branch
        if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
        {
            TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), TEXT("OnAllComplete"));
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
}

void UFlowNode::ExecuteBranch(USpellExecutionContext* Context)
{
    bool bCondition = true;
    
    // Get condition from input
    if (Context->HasVariable(TEXT("Condition")))
    {
        bCondition = Context->GetVariable(TEXT("Condition")).BoolValue;
    }
    
    FName PinName = bCondition ? TEXT("True") : TEXT("False");
    
    if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
    {
        TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), PinName);
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
    
    UE_LOG(LogTemp, Log, TEXT("Branch executed %s path for %s"), 
        bCondition ? TEXT("True") : TEXT("False"), *NodeName);
}

void UFlowNode::ExecuteGate(USpellExecutionContext* Context)
{
    bool bGateOpen = Context->HasVariable(TEXT("GateOpen")) ? 
                     Context->GetVariable(TEXT("GateOpen")).BoolValue : true;
    
    FName PinName = bGateOpen ? TEXT("Open") : TEXT("Closed");
    
    if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
    {
        TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), PinName);
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
    
    UE_LOG(LogTemp, Log, TEXT("Gate %s for %s"), 
        bGateOpen ? TEXT("opened") : TEXT("closed"), *NodeName);
}

void UFlowNode::InitializeLoopState(USpellExecutionContext* Context)
{
    CurrentLoopState.CurrentIteration = 0;
    CurrentLoopState.MaxIterations = MaxIterations;
    CurrentLoopState.bIsActive = true;
    CurrentLoopState.bShouldContinue = true;
    CurrentLoopState.IterationDelay = IterationDelay;
    
    // Override max iterations from input
    if (Context->HasVariable(TEXT("IterationCount")))
    {
        FGWTVariableValue CountVar = Context->GetVariable(TEXT("IterationCount"));
        if (CountVar.Type == EGWTVariableType::Int)
        {
            CurrentLoopState.MaxIterations = CountVar.IntValue;
        }
    }
    
    // Apply rarity scaling
    int32 MaxAllowed = GetMaxIterationsByRarity();
    CurrentLoopState.MaxIterations = FMath::Min(CurrentLoopState.MaxIterations, MaxAllowed);
}

bool UFlowNode::ShouldContinueLoop(USpellExecutionContext* Context)
{
    return CurrentLoopState.bShouldContinue &&
           CurrentLoopState.CurrentIteration < CurrentLoopState.MaxIterations &&
           (!bBreakOnCondition || !EvaluateBreakCondition(Context));
}

void UFlowNode::UpdateLoopState(USpellExecutionContext* Context)
{
    CurrentLoopState.CurrentIteration++;
    
    // Check for dynamic loop control
    if (Context->HasVariable(TEXT("ShouldContinue")))
    {
        CurrentLoopState.bShouldContinue = Context->GetVariable(TEXT("ShouldContinue")).BoolValue;
    }
}

void UFlowNode::HandleDelayComplete()
{
    if (CachedContext)
    {
        // Execute connected nodes after delay
        TArray<USpellNode*> NextNodes = GetConnectedOutputNodes();
        for (USpellNode* Node : NextNodes)
        {
            if (Node && IsValid(Node))
            {
                Node->Execute(CachedContext);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Delay completed for %s"), *NodeName);
    }
    
    DelayTimerHandle.Invalidate();
}

void UFlowNode::HandleIterationTimer()
{
    // Called for timed loop iterations
    if (CachedContext && ShouldContinueLoop(CachedContext))
    {
        TArray<USpellNode*> LoopBodyNodes = GetLoopBodyNodes();
        
        for (USpellNode* Node : LoopBodyNodes)
        {
            if (Node && IsValid(Node))
            {
                Node->Execute(CachedContext);
            }
        }
        
        UpdateLoopState(CachedContext);
        
        if (ShouldContinueLoop(CachedContext))
        {
            // Schedule next iteration
            UWorld* World = GetWorld();
            if (World)
            {
                World->GetTimerManager().SetTimer(
                    IterationTimerHandle,
                    this,
                    &UFlowNode::HandleIterationTimer,
                    CurrentLoopState.IterationDelay,
                    false
                );
            }
        }
    }
}

TArray<USpellNode*> UFlowNode::GetLoopBodyNodes() const
{
    TArray<USpellNode*> BodyNodes;
    
    if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
    {
        TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), TEXT("LoopBody"));
        for (const FHeartGraphPinReference& Connection : Connections)
        {
            if (UHeartGraphNode* Node = Graph->GetNode(Connection.NodeGuid))
            {
                if (USpellNode* SpellNode = Cast<USpellNode>(Node))
                {
                    BodyNodes.Add(SpellNode);
                }
            }
        }
    }
    
    return BodyNodes;
}

TArray<USpellNode*> UFlowNode::GetParallelNodes() const
{
    TArray<USpellNode*> ParallelNodes;
    
    if (UHeartGraph* Graph = GetTypedOuter<UHeartGraph>())
    {
        TArray<FName> BranchNames = {TEXT("Branch1"), TEXT("Branch2"), TEXT("Branch3")};
        
        for (const FName& BranchName : BranchNames)
        {
            TArray<FHeartGraphPinReference> Connections = Graph->GetConnectedPins(GetNodeGuid(), BranchName);
            for (const FHeartGraphPinReference& Connection : Connections)
            {
                if (UHeartGraphNode* Node = Graph->GetNode(Connection.NodeGuid))
                {
                    if (USpellNode* SpellNode = Cast<USpellNode>(Node))
                    {
                        ParallelNodes.Add(SpellNode);
                    }
                }
            }
        }
    }
    
    return ParallelNodes;
}

bool UFlowNode::EvaluateBreakCondition(USpellExecutionContext* Context)
{
    if (Context->HasVariable(BreakConditionVariable))
    {
        FGWTVariableValue BreakVar = Context->GetVariable(BreakConditionVariable);
        if (BreakVar.Type == EGWTVariableType::Bool)
        {
            return BreakVar.BoolValue;
        }
    }
    
    return false;
}

bool UFlowNode::CheckRecursionLimit(USpellExecutionContext* Context)
{
    RecursionDepth++;
    
    if (RecursionDepth > MaxRecursionDepth)
    {
        RecursionDepth = 0;
        return false;
    }
    
    // Reset recursion depth when execution completes
    if (Context->ExecutionDepth == 0)
    {
        RecursionDepth = 0;
    }
    
    return true;
}

int32 UFlowNode::GetMaxIterationsByRarity() const
{
    switch (NodeRarity)
    {
        case EItemRarity::Common:
            return 3;
        case EItemRarity::Uncommon:
            return 5;
        case EItemRarity::Rare:
            return 10;
        case EItemRarity::Epic:
            return 20;
        case EItemRarity::Legendary:
            return 50;
        default:
            return 3;
    }
}

void UFlowNode::ApplyRarityEffects(USpellExecutionContext* Context)
{
    switch (NodeRarity)
    {
        case EItemRarity::Uncommon:
            // Improved timing precision
            Context->SetVariable(TEXT("TimingPrecision"), FGWTVariableValue::FromFloat(0.9f));
            break;
            
        case EItemRarity::Rare:
            // Loop condition optimization
            Context->SetVariable(TEXT("OptimizedLoop"), FGWTVariableValue::FromBool(true));
            break;
            
        case EItemRarity::Epic:
            // Parallel execution support
            Context->SetVariable(TEXT("ParallelSupport"), FGWTVariableValue::FromBool(true));
            break;
            
        case EItemRarity::Legendary:
            // Quantum flow - can execute multiple paths simultaneously
            Context->SetVariable(TEXT("QuantumFlow"), FGWTVariableValue::FromBool(true));
            break;
    }
}

float UFlowNode::GetBasePower() const
{
    return 0.0f; // Flow nodes don't provide direct power
}