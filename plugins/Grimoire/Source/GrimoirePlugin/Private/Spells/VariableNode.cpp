// Source/GrimoirePlugin/Private/VariableNode.cpp
#include "VariableNode.h"

void UVariableNode::Execute(UObject* Context)
{
    Super::Execute(Context);

    // Store or retrieve value; propagate to outputs
    UE_LOG(LogTemp, Log, TEXT("Variable %s Accessed"), *VariableName);
}

float UVariableNode::GetBasePower() const
{
    return 0.0f; // Variables don't have power
}

void UVariableNode::SetValueFromInput(UObject* InputValue)
{
    switch (ValueType)
    {
        case EVariableType::Float:
            if (float* Val = CastField<float>(InputValue))
                FloatValue = *Val;
            break;
        case EVariableType::Bool:
            if (bool* Val = CastField<bool>(InputValue))
                BoolValue = *Val;
            break;
        case EVariableType::Int:
            if (int32* Val = CastField<int32>(InputValue))
                IntValue = *Val;
            break;
        case EVariableType::String:
            if (FString* Val = CastField<FString>(InputValue))
                StringValue = *Val;
            break;
        default:
            break;
    }
}