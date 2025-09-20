// Source/GrimoirePlugin/Private/FlowNode.cpp
#include "FlowNode.h"

void UFlowNode::Execute(UObject* Context)
{
    Super::Execute(Context);

    if (RecursionDepth > MaxRecursion)
    {
        UE_LOG(LogTemp, Error, TEXT("Flow Node Recursion Limit Reached"));
        return;
    }
    RecursionDepth++;

    switch (LoopType)
    {
        case ELoopType::Repeat:
            for (int32 i = 0; i < IterationCount; i++)
            {
                for (UHeartGraphPin* Pin : OutputPins)
                {
                    if (Pin && Pin->IsConnected())
                    {
                        USpellNode* Target = Cast<USpellNode>(Pin->GetConnectedNode());
                        if (Target)
                        {
                            Target->Execute(Context);
                        }
                    }
                }
            }
            break;
        case ELoopType::While:
            while (LoopCondition)
            {
                for (UHeartGraphPin* Pin : OutputPins)
                {
                    if (Pin && Pin->IsConnected())
                    {
                        USpellNode* Target = Cast<USpellNode>(Pin->GetConnectedNode());
                        if (Target)
                        {
                            Target->Execute(Context);
                        }
                    }
                }
                // Update condition (example: decrement counter)
                IterationCount--;
                LoopCondition = (IterationCount > 0);
            }
            break;
        case ELoopType::For:
            for (int32 i = 0; i < IterationCount; i++)
            {
                for (UHeartGraphPin* Pin : OutputPins)
                {
                    if (Pin && Pin->IsConnected())
                    {
                        USpellNode* Target = Cast<USpellNode>(Pin->GetConnectedNode());
                        if (Target)
                        {
                            Target->Execute(Context);
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
    RecursionDepth--;
}

float UFlowNode::GetBasePower() const
{
    return 0.0f; // Flows don't have power
}