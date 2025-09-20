#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "InputAction.h"
#include "GrimoireTypes.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESpellElement : uint8
{
    Fire,
    Water,
    Earth,
    Electricity,
    Ice,
    Plant,
    Metal,
    Poison,
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESpellNodeType : uint8
{
    Magic     UMETA(DisplayName = "Magic"),
    Trigger   UMETA(DisplayName = "Trigger"),
    Effect    UMETA(DisplayName = "Effect"),
    Condition UMETA(DisplayName = "Condition"),
    Variable  UMETA(DisplayName = "Variable"),
    Flow      UMETA(DisplayName = "Flow"),
    MAX       UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETriggerType : uint8
{
    ManualKey     UMETA(DisplayName = "Manual Key Press"),
    AutoEvent     UMETA(DisplayName = "Automatic Event"),
    OnHit         UMETA(DisplayName = "On Hit"),
    OnCast        UMETA(DisplayName = "On Cast"),
    MAX           UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EGWTAbilityInputID : uint8
{
    None,
    Spell1,
    Spell2,
    Spell3,
    Spell4,
    Spell5,
    MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FNodeDefinition : public FTableRowBase  // For modding: DataTable of node templates
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<USpellNode> NodeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DefaultDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int DefaultManaCost = 10;
};

USTRUCT(BlueprintType)
struct FSpellPortDescriptor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Port")
    FName PortName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Port")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Port")
    ESpellNodeType AcceptedType;
};

USTRUCT(BlueprintType)
struct FElementInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    ESpellElement SourceElement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    ESpellElement TargetElement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText EffectDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCreatesSustainedEffect = false;
};

USTRUCT(BlueprintType)
struct FElementInteractionRow : public FTableRowBase
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FElementInteraction> Interactions;
};

UCLASS()
class GRIMOIREPLUGIN_API UNodeDataAsset : public UDataAsset  // modding: Load custom nodes
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UDataTable* NodeDefinitions;
};
