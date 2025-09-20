// Source/GrimoirePlugin/Public/VariableNode.h
#pragma once

#include "CoreMinimal.h"
#include "SpellNode.h"
#include "VariableNode.generated.h"

UENUM(BlueprintType)
enum class EVariableType : uint8
{
    Float,
    Bool,
    Int,
    String,
    MAX UMETA(Hidden)
};

UCLASS(Blueprintable, meta = (DisplayName = "Variable Node"))
class GRIMOIREPLUGIN_API UVariableNode : public USpellNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    FString VariableName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    EVariableType ValueType = EVariableType::Float;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    float FloatValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    bool BoolValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    int32 IntValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    FString StringValue;

    virtual void Execute(UObject* Context) override;
    virtual float GetBasePower() const override;

    UFUNCTION(BlueprintCallable, Category = "Variable")
    void SetValueFromInput(UObject* InputValue);
};