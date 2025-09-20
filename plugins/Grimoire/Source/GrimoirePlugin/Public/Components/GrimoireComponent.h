#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Spells/SpellNode.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Model/HeartGraph.h"
#include "GrimoireComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GRIMOIREPLUGIN_API UGrimoireComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGrimoireComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Spell management
    UFUNCTION(BlueprintCallable, Category = "Grimoire")
    void AddSpellNode(TSubclassOf<USpellNode> SpellNodeClass);

    UFUNCTION(BlueprintCallable, Category = "Grimoire")
    void RemoveSpellNode(USpellNode* SpellNode);

    UFUNCTION(BlueprintCallable, Category = "Grimoire")
    void ExecuteSpell(FName SpellName);

    // Compile and grant GAS ability
    UFUNCTION(BlueprintCallable, Category = "GAS")
    void CompileAndGrantSpellAbility(FName SpellName, int32 InputID);

    UFUNCTION(Server, Reliable)
    void Server_ExecuteSpell(FName SpellName);

    UFUNCTION(Client, Reliable)
    void Client_PredictExecute(FName SpellName);

    // Node collection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grimoire", replicated)
    TArray<USpellNode*> AvailableNodes;

    // Active spells
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grimoire", replicated)
    TMap<FName, USpellNode*> ActiveSpells;

    // Mana management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grimoire", ReplicatedUsing=OnRep_CurrentMana)
    float CurrentMana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grimoire")
    float MaxMana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grimoire")
    float ManaRegenRate;
    
    // Spell properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grimoire")
    float BaseDamage;
	
    // Replication callbacks
    UFUNCTION()
    void OnRep_CurrentMana();

    // GAS Component 
    UPROPERTY()
    UAbilitySystemComponent* AbilitySystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputMappingContext* InputContext;  // For Enhanced Input

    USpellNode* FindRootNode(UHeartGraph* Graph) const;

private:
    bool CanCastSpell(const USpellNode* SpellNode) const;
    void ConsumeMana(float Amount);
    float CalculateSpellManaCost(UHeartGraph* Graph) const;
};