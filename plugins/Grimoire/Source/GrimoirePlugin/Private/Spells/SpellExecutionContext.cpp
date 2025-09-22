// Source/GrimoirePlugin/Private/SpellExecutionContext.cpp
#include "SpellExecutionContext.h"
#include "Engine/World.h"

// Variable value conversion helpers
FVariableValue FVariableValue::FromFloat(float Value)
{
    FVariableValue Result;
    Result.Type = EVariableType::Float;
    Result.FloatValue = Value;
    return Result;
}

FVariableValue FVariableValue::FromInt(int32 Value)
{
    FVariableValue Result;
    Result.Type = EVariableType::Int;
    Result.IntValue = Value;
    return Result;
}

FVariableValue FVariableValue::FromBool(bool Value)
{
    FVariableValue Result;
    Result.Type = EVariableType::Bool;
    Result.BoolValue = Value;
    return Result;
}

FVariableValue FVariableValue::FromString(const FString& Value)
{
    FVariableValue Result;
    Result.Type = EVariableType::String;
    Result.StringValue = Value;
    return Result;
}

FVariableValue FVariableValue::FromVector(const FVector& Value)
{
    FVariableValue Result;
    Result.Type = EVariableType::Vector;
    Result.VectorValue = Value;
    return Result;
}

FVariableValue FVariableValue::FromActor(AActor* Value)
{
    FVariableValue Result;
    Result.Type = EVariableType::Object;
    Result.ObjectValue = Value;
    return Result;
}

// Context implementation
void USpellExecutionContext::SetVariable(FName Name, const FVariableValue& Value, bool bGlobal)
{
    if (bGlobal)
    {
        GlobalVariables.Add(Name, Value);
    }
    else
    {
        LocalVariables.Add(Name, Value);
    }
}

FVariableValue USpellExecutionContext::GetVariable(FName Name, bool bGlobal) const
{
    const FVariableValue* Value = nullptr;
    
    if (bGlobal)
    {
        Value = GlobalVariables.Find(Name);
    }
    else
    {
        Value = LocalVariables.Find(Name);
        // Fall back to global if not found locally
        if (!Value)
        {
            Value = GlobalVariables.Find(Name);
        }
    }
    
    return Value ? *Value : FVariableValue();
}

bool USpellExecutionContext::HasVariable(FName Name, bool bGlobal) const
{
    if (bGlobal)
    {
        return GlobalVariables.Contains(Name);
    }
    else
    {
        return LocalVariables.Contains(Name) || GlobalVariables.Contains(Name);
    }
}

void USpellExecutionContext::ClearVariables(bool bGlobal)
{
    if (bGlobal)
    {
        GlobalVariables.Empty();
    }
    else
    {
        LocalVariables.Empty();
    }
}

USpellExecutionContext* USpellExecutionContext::CreateChildContext() const
{
    USpellExecutionContext* ChildContext = NewObject<USpellExecutionContext>();
    ChildContext->Caster = Caster;
    ChildContext->Target = Target;
    ChildContext->TargetLocation = TargetLocation;
    ChildContext->HitResult = HitResult;
    ChildContext->GlobalVariables = GlobalVariables; // Copy global variables
    ChildContext->ExecutionTime = ExecutionTime;
    ChildContext->ExecutionDepth = ExecutionDepth + 1;
    
    return ChildContext;
}

void USpellExecutionContext::MergeChildContext(const USpellExecutionContext* ChildContext)
{
    // Merge global variables (child can modify globals)
    GlobalVariables = ChildContext->GlobalVariables;
}

FString USpellExecutionContext::GetDebugString() const
{
    return FString::Printf(TEXT("SpellContext [Caster: %s, Target: %s, Vars: %d local, %d global]"),
        Caster ? *Caster->GetName() : TEXT("None"),
        Target ? *Target->GetName() : TEXT("None"),
        LocalVariables.Num(),
        GlobalVariables.Num());
}