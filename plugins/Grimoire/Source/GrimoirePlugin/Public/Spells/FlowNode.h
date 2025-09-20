// Source/GrimoirePlugin/Public/FlowNode.h
#pragma once

#include "CoreMinimal.h"
#include "SpellNode.h"
#include "FlowNode.generated.h"

UENUM(BlueprintType)
enum class ELoopType : uint8
{
    Repeat,
    While,
    For,
    MAX UMETA(Hidden)
};

UCLASS(Blueprintable, meta = (DisplayName = "Flow Node"))
class GRIMOIREPLUGIN_API UFlowNode : public USpellNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
    ELoopType LoopType = ELoopType::Repeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
    int32 IterationCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
    bool LoopCondition = true; // For While

    virtual void Execute(UObject* Context) override;
    virtual float GetBasePower() const override;

protected:
    int32 RecursionDepth = 0;
    const int32 MaxRecursion = 10; // Prevent stack overflow
};