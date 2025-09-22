#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Spells/SpellNode.h"
#include "SpellDataAsset.generated.h"

UCLASS(BlueprintType)
class GRIMOIREPLUGIN_API USpellDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell")
	TArray<TObjectPtr<USpellNode>> NodeInstances;

	/** Return runtime node instances (cloned for safety) */
	UFUNCTION(BlueprintCallable, Category = "Spell")
	TArray<TObjectPtr<USpellNode>> GetRuntimeNodes() const
	{
		return NodeInstances; // for MVP return references; later clone instances here
	}
};
