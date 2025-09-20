// Source/GrimoirePlugin/Private/SpellExecutionContext.cpp
#include "SpellExecutionContext.h"
#include "Engine/World.h"

// Variable value conversion helpers
FGWTVariableValue FGWTVariableValue::FromFloat(float Value)
{
    FGWTVariableValue Result;
    Result.Type = EGWTVariableType::Float;
    Result.FloatValue = Value;
    return Result;
}

FGWTVariableValue FGWTVariableValue::FromInt(int32 Value)
{
    FGWTVariableValue Result;
    Result.Type = EGWTVariableType::Int;
    Result.IntValue = Value;
    return Result;
}

FGWTVariableValue FGWTVariableValue::FromBool(bool Value)
{
    FGWTVariableValue Result;
    Result.Type = EGWTVariableType::Bool;
    Result.BoolValue = Value;
    return Result;
}

FGWTVariableValue FGWTVariableValue::FromString(const FString& Value)
{
    FGWTVariableValue Result;
    Result.Type = EGWTVariableType::String;
    Result.StringValue = Value;
    return Result;
}

FGWTVariableValue FGWTVariableValue::FromVector(const FVector& Value)
{
    FGWTVariableValue Result;
    Result.Type = EGWTVariableType::Vector;
    Result.VectorValue = Value;
    return Result;
}

FGWTVariableValue FGWTVariableValue::FromActor(AActor* Value)
{
    FGWTVariableValue Result;
    Result.Type = EGWTVariableType::Object;
    Result.ObjectValue = Value;
    return Result;
}

// Context implementation
void USpellExecutionContext::SetVariable(FName Name, const FGWTVariableValue& Value, bool bGlobal)
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

FGWTVariableValue USpellExecutionContext::GetVariable(FName Name, bool bGlobal) const
{
    const FGWTVariableValue* Value = nullptr;
    
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
    
    return Value ? *Value : FGWTVariableValue();
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