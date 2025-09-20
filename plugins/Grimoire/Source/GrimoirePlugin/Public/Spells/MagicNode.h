#pragma once

#include "CoreMinimal.h"
#include "Spells/SpellNode.h"
#include "MagicNode.generated.h"

UCLASS(Blueprintable, meta = (DisplayName = "Magic Node"))
class GRIMOIREPLUGIN_API UMagicNode : public USpellNode
{
    GENERATED_BODY()

public:
    UMagicNode();

    // properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic")
    float BaseDamage = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic")
    ESpellElement ElementType = ESpellElement::Fire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic")
    float Range = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic")
    float ManaCost = 20.0f;

    // Execution
    virtual void Execute(UGrimoireComponent* Grimoire, AActor* ContextActor) override;
    virtual float GetBasePower() const override;

private:
    void ApplyDamage(AActor* Target, float DamageAmount);
    void ApplyElementalEffects(AActor* Target);
};