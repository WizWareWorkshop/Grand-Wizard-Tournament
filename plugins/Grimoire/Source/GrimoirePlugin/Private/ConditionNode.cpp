// Source/GrimoirePlugin/Private/ConditionNode.cpp
#include "ConditionNode.h"
#include "Math/UnrealMathUtility.h"

void UConditionNode::Execute(UObject* Context)
{
    Super::Execute(Context);

    if (EvaluateCondition(Context))
    {
        // Propagate to all outputs (true branch)
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
    // No false branch; single branch for simplicity; subject to change
}

float UConditionNode::GetBasePower() const
{
    return 0.0f; // Conditions don't have power
}

bool UConditionNode::EvaluateCondition(UObject* Context)
{
    switch (ConditionType)
    {
        case EConditionType::IfThen:
            // Example: Check health < 50% (assume context has health)
            return FMath::RandBool(); // Placeholder logic; replace with context data
        case EConditionType::Threshold:
            float Value = 0.6f; // From input or context
            return Value > Threshold;
        case EConditionType::Random:
            return FMath::FRand() < Threshold;
        default:
            return false;
    }
}