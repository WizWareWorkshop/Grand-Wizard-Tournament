#include "Spells/MagicNode.h"
#include "model/HeartGraphPin.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h" // For Niagara effects

void UMagicNode::Execute(UGrimoireComponent* Grimoire, AActor* ContextActor)
{
    Super::Execute(Grimoire, ContextActor);

    UWorld* World = ContextActor ? ContextActor->GetWorld() : nullptr;
    if (!World) return;

    FVector SpawnLocation = FVector::ZeroVector; // From context or caster
    FRotator SpawnRotation = FRotator::ZeroRotator;
    AActor* Projectile = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
    if (Projectile)
    {
        Projectile->SetLifeSpan(Range / 1000.0f);
        UE_LOG(LogTemp, Log, TEXT("Magic Node: Spawned Projectile with Element %s"), *UEnum::GetValueAsString(ElementType));
    }
}

float UMagicNode::GetBasePower() const
{
    return BaseDamage * GetRarityScaleFactor();
}

void UMagicNode::ApplyDamage(AActor* Target, float DamageAmount)
{
    UGameplayStatics::ApplyDamage(Target, DamageAmount, nullptr, GetOwner(), UDamageType::StaticClass());
}

void UMagicNode::OnPinCollision(UHeartGraphPin* OtherPin, UObject* Context)
{
    if (!OtherPin) return;

    UMagicNode* OtherNode = Cast<UMagicNode>(OtherPin->GetOwningNode());
    if (OtherNode)
    {
        FElementInteraction Interaction = GetElementInteraction(ElementType, OtherNode->ElementType);
        UWorld* World = Context->GetWorld();
        if (World && Interaction.bCreatesSustainedEffect)
        {
            FVector CollisionLocation = FVector::ZeroVector; // From collision data
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, LoadObject<UNiagaraSystem>(nullptr, TEXT("/Content/Niagara/NS_GenericEffect.niagara")), CollisionLocation); // Example Niagara
            UE_LOG(LogTemp, Log, TEXT("Interaction: %s - Duration %.2f"), *Interaction.EffectDescription.ToString(), Interaction.Duration);
        }
        float Damage = BaseDamage * Interaction.DamageMultiplier;
        UGameplayStatics::ApplyDamage(Context, Damage, nullptr, nullptr, UDamageType::StaticClass());
    }
}

FElementInteraction UMagicNode::GetElementInteraction(EElementType Source, EElementType Target)
{
    // Full table loaded from data asset
    static UDataTable* InteractionTable = LoadObject<UDataTable>(nullptr, TEXT("/Content/ElementalInteractionsDataTable.ElementalInteractionsDataTable"));
    if (InteractionTable)
    {
        FName RowName = FName(*FString::Printf(TEXT("%s_%s"), *UEnum::GetValueAsString(Source), *UEnum::GetValueAsString(Target)));
        FElementInteraction* Interaction = InteractionTable->FindRow<FElementInteraction>(RowName, TEXT(""));
        if (Interaction)
            return *Interaction;
    }
    return FElementInteraction{ Source, Target, FText::FromString("No interaction"), 1.0f, 0.0f, false };
}