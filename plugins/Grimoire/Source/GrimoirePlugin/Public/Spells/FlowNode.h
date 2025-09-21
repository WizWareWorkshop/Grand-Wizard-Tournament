#pragma once

#include "CoreMinimal.h"
#include "Spells/SpellNode.h"
#include "Engine/TimerHandle.h"
#include "FlowNode.generated.h"

UENUM(BlueprintType)
enum class EFlowNodeType : uint8
{
    Sequence        UMETA(DisplayName = "Sequence"),
    Loop            UMETA(DisplayName = "Loop"),
    WhileLoop       UMETA(DisplayName = "While Loop"),
    ForLoop         UMETA(DisplayName = "For Loop"),
    Delay           UMETA(DisplayName = "Delay"),
    Parallel        UMETA(DisplayName = "Parallel"),
    Branch          UMETA(DisplayName = "Branch"),
    Gate            UMETA(DisplayName = "Gate"),
    MAX             UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FLoopState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentIteration = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxIterations = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShouldContinue = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IterationDelay = 0.1f;
};

UCLASS(Blueprintable, meta = (DisplayName = "Flow Node"))
class GRIMOIREPLUGIN_API UFlowNode : public USpellNode
{
    GENERATED_BODY()

public:
    UFlowNode();

    // Flow properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
    EFlowNodeType FlowType = EFlowNodeType::Sequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow", meta = (ClampMin = "1"))
    int32 MaxIterations = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow", meta = (ClampMin = "0.0"))
    float DelayTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow", meta = (ClampMin = "0.0"))
    float IterationDelay = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
    bool bBreakOnCondition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
    FName BreakConditionVariable = TEXT("ShouldBreak");

    // Execution
    virtual void OnExecute(USpellExecutionContext* Context) override;
    virtual float GetBasePower() const override;

    // Pin system override for flow-specific pins
    virtual TArray<FHeartGraphPinDesc> GetInputPinDescs() const override;
    virtual TArray<FHeartGraphPinDesc> GetOutputPinDescs() const override;

    // Flow control
    UFUNCTION(BlueprintCallable, Category = "Flow")
    void ExecuteSequence(USpellExecutionContext* Context);

    UFUNCTION(BlueprintCallable, Category = "Flow")
    void ExecuteLoop(USpellExecutionContext* Context);

    UFUNCTION(BlueprintCallable, Category = "Flow")
    void ExecuteDelay(USpellExecutionContext* Context);

    UFUNCTION(BlueprintCallable, Category = "Flow")
    void ExecuteParallel(USpellExecutionContext* Context);

protected:
    // Flow execution methods
    void ExecuteSequenceInternal(USpellExecutionContext* Context);
    void ExecuteLoopInternal(USpellExecutionContext* Context);
    void ExecuteWhileLoop(USpellExecutionContext* Context);
    void ExecuteForLoop(USpellExecutionContext* Context);
    void ExecuteDelayInternal(USpellExecutionContext* Context);
    void ExecuteParallelInternal(USpellExecutionContext* Context);
    void ExecuteBranch(USpellExecutionContext* Context);
    void ExecuteGate(USpellExecutionContext* Context);

    // Loop management
    void InitializeLoopState(USpellExecutionContext* Context);
    bool ShouldContinueLoop(USpellExecutionContext* Context);
    void UpdateLoopState(USpellExecutionContext* Context);

    // Timer callbacks
    UFUNCTION()
    void HandleDelayComplete();

    UFUNCTION()
    void HandleIterationTimer();

    // Utility functions
    TArray<USpellNode*> GetLoopBodyNodes() const;
    TArray<USpellNode*> GetParallelNodes() const;
    bool EvaluateBreakCondition(USpellExecutionContext* Context);

    // Rarity effects
    void ApplyRarityEffects(USpellExecutionContext* Context);

    // Recursion protection
    bool CheckRecursionLimit(USpellExecutionContext* Context);

private:
    // Flow state
    UPROPERTY()
    FLoopState CurrentLoopState;

    UPROPERTY()
    USpellExecutionContext* CachedContext;

    // Timer handles
    UPROPERTY()
    FTimerHandle DelayTimerHandle;

    UPROPERTY()
    FTimerHandle IterationTimerHandle;

    // Recursion tracking
    int32 RecursionDepth = 0;
    static const int32 MaxRecursionDepth = 50;

    // Parallel execution tracking
    UPROPERTY()
    TArray<USpellNode*> ActiveParallelNodes;

    int32 CompletedParallelNodes = 0;
};