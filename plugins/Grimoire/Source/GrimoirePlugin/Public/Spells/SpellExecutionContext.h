#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "SpellExecutionContext.generated.h"

/** Lightweight variable container for nodes */
UENUM(BlueprintType)
enum class ESpellVariableType : uint8
{
    None,
    Float,
    Int,
    Bool,
    Vector,
    Actor
};

USTRUCT(BlueprintType)
struct FSpellVariableValue
{
    GENERATED_BODY()

    UPROPERTY()
    ESpellVariableType Type = ESpellVariableType::None;

    UPROPERTY()
    float FloatValue = 0.0f;

    UPROPERTY()
    int32 IntValue = 0;

    UPROPERTY()
    bool BoolValue = false;

    UPROPERTY()
    FVector VectorValue = FVector::ZeroVector;

    UPROPERTY()
    TWeakObjectPtr<AActor> ActorValue = nullptr;

    static FSpellVariableValue FromFloat(float In) { FSpellVariableValue V; V.Type = ESpellVariableType::Float; V.FloatValue = In; return V; }
    static FSpellVariableValue FromInt(int32 In) { FSpellVariableValue V; V.Type = ESpellVariableType::Int; V.IntValue = In; return V; }
    static FSpellVariableValue FromBool(bool In) { FSpellVariableValue V; V.Type = ESpellVariableType::Bool; V.BoolValue = In; return V; }
    static FSpellVariableValue FromVector(const FVector& In) { FSpellVariableValue V; V.Type = ESpellVariableType::Vector; V.VectorValue = In; return V; }
    static FSpellVariableValue FromActor(AActor* In) { FSpellVariableValue V; V.Type = ESpellVariableType::Actor; V.ActorValue = In; return V; }
};

UCLASS(BlueprintType)
class GRIMOIREPLUGIN_API USpellExecutionContext : public UObject
{
    GENERATED_BODY()

public:
    USpellExecutionContext();

    UPROPERTY()
    TWeakObjectPtr<AActor> Caster;

    UPROPERTY()
    TWeakObjectPtr<AActor> Target;

    UPROPERTY()
    TMap<FName, FSpellVariableValue> Variables;

    UFUNCTION(BlueprintCallable)
    void SetVariable(FName Key, const FSpellVariableValue& Value);

    UFUNCTION(BlueprintCallable)
    bool HasVariable(FName Key) const;

    UFUNCTION(BlueprintCallable)
    FSpellVariableValue GetVariable(FName Key) const;

    UFUNCTION(BlueprintCallable)
    USpellExecutionContext* DuplicateContext() const;
};
