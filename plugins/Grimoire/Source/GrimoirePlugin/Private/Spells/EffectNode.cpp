// Source/GrimoirePlugin/Private/EffectNode.cpp
#include "EffectNode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void UEffectNode::Execute(UObject* Context)
{
    Super::Execute(Context);

    float Power = GetBasePower() * Intensity;

    switch (EffectType)
    {
        case EEffectType::Damage:
            ApplyDamage(Context, Power);
            break;
        case EEffectType::Teleport:
            ApplyTeleport(Context);
            break;
        case EEffectType::Knockback:
            ApplyKnockback(Context);
            break;
        case EEffectType::Heal:
            ApplyHeal(Context, Power);
            break;
        case EEffectType::StatusEffect:
            ApplyStatusEffect(Context);
            break;
        default:
            break;
    }
}

float UEffectNode::GetBasePower() const
{
    return Intensity * GetRarityScaleFactor();
}

void UEffectNode::ApplyDamage(UObject* Context, float DamageAmount)
{
    ACharacter* Target = Cast<ACharacter>(Context);
    if (Target)
    {
        UGameplayStatics::ApplyDamage(Target, DamageAmount, nullptr, nullptr, UDamageType::StaticClass());
        UE_LOG(LogTemp, Log, TEXT("Effect: Applied %.2f Damage"), DamageAmount);
    }
}

void UEffectNode::ApplyTeleport(UObject* Context)
{
    ACharacter* Target = Cast<ACharacter>(Context);
    if (Target)
    {
        FVector NewLocation = Target->GetActorLocation() + FVector(0, 0, 100.0f); // Example teleport up
        Target->SetActorLocation(NewLocation);
        UE_LOG(LogTemp, Log, TEXT("Effect: Teleported to %s"), *NewLocation.ToString());
    }
}

void UEffectNode::ApplyKnockback(UObject* Context)
{
    ACharacter* Target = Cast<ACharacter>(Context);
    if (Target)
    {
        FVector KnockbackForce = FVector(0, 0, 300.0f); // Example upward knockback
        Target->LaunchCharacter(KnockbackForce, false, false);
        UE_LOG(LogTemp, Log, TEXT("Effect: Applied Knockback"));
    }
}

void UEffectNode::ApplyHeal(UObject* Context, float HealAmount)
{
    ACharacter* Target = Cast<ACharacter>(Context);
    if (Target)
    {
        Target->ModifyHealth(HealAmount); // Assume custom health method
        UE_LOG(LogTemp, Log, TEXT("Effect: Healed %.2f Health"), HealAmount);
    }
}

void UEffectNode::ApplyStatusEffect(UObject* Context)
{
    ACharacter* Target = Cast<ACharacter>(Context);
    if (Target)
    {
        // Apply debuff based on StatusType (e.g., Burning: DoT)
        UE_LOG(LogTemp, Log, TEXT("Effect: Applied %s for %.2f seconds"), *UEnum::GetValueAsString(StatusType), StatusDuration);
        // Timer for duration (similar to TriggerNode)
    }
}