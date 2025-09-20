// Source/GrimoirePlugin/Public/TriggerNode.h
#pragma once

#include "CoreMinimal.h"
#include "SpellNode.h"
#include "TriggerNode.generated.h"

UENUM(BlueprintType)
enum class ETriggerEventType : uint8
{
    OnCast,
    OnHit,
    OnEnemyEnter,
    OnTimer,
    MAX UMETA(Hidden)
};

UCLASS(Blueprintable, meta = (DisplayName = "Trigger Node"))
class GRIMOIREPLUGIN_API UTriggerNode : public USpellNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    ETriggerEventType EventType = ETriggerEventType::OnCast;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float TriggerRange = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float TimerInterval = 0.0f; // For OnTimer

    virtual void Execute(UObject* Context) override;
    virtual float GetBasePower() const override;

protected:
    FTimerHandle TimerHandle;

    UFUNCTION()
    void HandleTimerTrigger(UObject* Context);
};