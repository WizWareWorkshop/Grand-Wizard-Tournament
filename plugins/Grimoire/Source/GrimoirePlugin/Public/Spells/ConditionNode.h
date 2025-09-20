// Source/GrimoirePlugin/Public/ConditionNode.h
#pragma once

#include "CoreMinimal.h"
#include "SpellNode.h"
#include "ConditionNode.generated.h"

UENUM(BlueprintType)
enum class EConditionType : uint8
{
    IfThen,
    Threshold,
    Random,
    MAX UMETA(Hidden)
};

UCLASS(Blueprintable, meta = (DisplayName = "Condition Node"))
class GRIMOIREPLUGIN_API UConditionNode : public USpellNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    EConditionType ConditionType = EConditionType::IfThen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    float Threshold = 0.5f; // For Threshold/Random

    virtual void Execute(UObject* Context) override;
    virtual float GetBasePower() const override;

protected:
    bool EvaluateCondition(UObject* Context);
};