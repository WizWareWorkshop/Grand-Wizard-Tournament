#pragma once

#include "CoreMinimal.h"
#include "Spells/SpellNode.h"
#include "GrimoireTypes.h"
#include "MagicNode.generated.h"

UCLASS(Blueprintable, meta = (DisplayName = "Magic Node"))
class GRIMOIREPLUGIN_API UMagicNode : public USpellNode
{
    GENERATED_BODY()

public:
    UMagicNode();

    // properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic", meta = (ClampMin = "0.0"))
    float BaseDamage = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic")
    ESpellElement ElementType = ESpellElement::Fire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic", meta = (ClampMin = "0.0"))
    float Range = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic")
    float ManaCost = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic")
    float Cooldown = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic", meta = (ClampMin = "0.0"))
    float CastTime = 1.0f;

    // Execution
    virtual void OnExecute(UGrimoireComponent* Grimoire, AActor* ContextActor) override;
    virtual float GetBasePower() const override;

    virtual TArray<FHeartGraphPinDesc> GetInputPinDescs() const override;
    virtual TArray<FHeartGraphPinDesc> GetOutputPinDescs() const override;

protected:
    // Magic-specific functionality
    void ApplyDamage(USpellExecutionContext* Context, float DamageAmount);
    void ApplyElementalEffects(USpellExecutionContext* Context);
    void CreateVisualEffects(USpellExecutionContext* Context);
    
    // Element interactions
    float GetElementalDamageMultiplier(AActor* Target) const;
    void ApplyElementalStatusEffect(AActor* Target, float Duration) const;
    
    // Rarity-based enhancements
    void ApplyRarityEffects(USpellExecutionContext* Context);
};