#pragma once

#include "CoreMinimal.h"
#include "Model/HeartGraphNode.h"
#include "Model/HeartGraphTypes.h"
#include "Model/HeartGraphPinDesc.h"
#include "Model/HeartGraphPinReference.h"
#include "GrimoireTypes.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "SpellNode.generated.h"

class UGrimoireComponent;
class UGameplayAbility;
class UInputAction;

UCLASS(Abstract, Blueprintable)
class GRIMOIREPLUGIN_API USpellNode : public UHeartGraphNode
{
    GENERATED_BODY()

public:
    USpellNode();

    // Node properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    FString NodeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    FVector2D Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node", Replicated)
    EItemRarity NodeRarity = EItemRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
    FLinearColor NodeColor = FLinearColor::White;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    FGuid NodeID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    FText NodeIcon = FText::FromString(TEXT("Icon"));

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    ESpellNodeType NodeType = ESpellNodeType::Magic;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    FText NodeCategory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    FText NodeDescription;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    int NodeCost;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Node")
    int NodeManaCost;

    // Execution methods
    UFUNCTION(BlueprintCallable, Category = "Execution")
    virtual void Execute(UGrimoireComponent* Grimoire, AActor* ContextActor);

    UFUNCTION(BlueprintCallable, Category = "Execution")
    virtual float GetBasePower() const;

    // HeartGraph Integration
    virtual void PostInitProperties() override;
    virtual TArray<FHeartGraphPinDesc> GetPins(EHeartPinDirection Direction) const override;

    // Rarity system
    UFUNCTION(BlueprintCallable, Category = "Rarity")
    float GetRarityScaleFactor() const;

    UFUNCTION(BlueprintCallable, Category = "Rarity")
    int32 GetMaxInputConnections() const;

    UFUNCTION(BlueprintCallable, Category = "Rarity")
    int32 GetMaxOutputConnections() const;

	// GAS compile to ability
    UFUNCTION(BlueprintCallable, Category = "GAS")
    virtual UGameplayAbility* CompileToGASAbility(float& OutSpellManaCost, int32 OutInputID) const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // HeartGraph pin initialization
    virtual void InitializeDefaultPins();
    
    // Execution helpers
    virtual void OnExecute(UGrimoireComponent* Grimoire, AActor* ContextActor);
    virtual void OnExecutionComplete(AActor* ContextActor);
    
    // Connection helpers
    TArray<USpellNode*> GetConnectedSpellNodes() const;

private:
    // Blood data integration for spell parameters
    TArray<FHeartGraphPinReference> InputPins;
    TArray<FHeartGraphPinReference> OutputPins;
};