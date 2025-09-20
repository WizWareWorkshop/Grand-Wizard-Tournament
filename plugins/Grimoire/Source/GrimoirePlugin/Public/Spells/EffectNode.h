// Source/GrimoirePlugin/Public/EffectNode.h
#pragma once

#include "CoreMinimal.h"
#include "SpellNode.h"
#include "EffectNode.generated.h"

UENUM(BlueprintType)
enum class EEffectType : uint8
{
    Damage,
    Teleport,
    Knockback,
    Heal,
    StatusEffect,
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
    Burning,
    Frozen,
    Poisoned,
    Stunned,
    MAX UMETA(Hidden)
};

UCLASS(Blueprintable, meta = (DisplayName = "Effect Node"))
class GRIMOIREPLUGIN_API UEffectNode : public USpellNode
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EEffectType EffectType = EEffectType::Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EStatusEffectType StatusType = EStatusEffectType::Burning; // If EffectType = StatusEffect

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float StatusDuration = 5.0f;

    virtual void Execute(UObject* Context) override;
    virtual float GetBasePower() const override;

protected:
    void ApplyDamage(UObject* Context, float DamageAmount);
    void ApplyTeleport(UObject* Context);
    void ApplyKnockback(UObject* Context);
    void ApplyHeal(UObject* Context, float HealAmount);
    void ApplyStatusEffect(UObject* Context);
};