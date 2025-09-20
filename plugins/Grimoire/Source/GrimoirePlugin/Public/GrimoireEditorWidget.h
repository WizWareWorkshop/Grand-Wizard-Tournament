#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Spells/SpellNode.h"
#include "GrimoireEditorWidget.generated.h"

class UHeartGraph;
class UPanelWidget;

UCLASS()
class GRIMOIREPLUGIN_API UGrimoireEditorWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Grimoire Editor")
    void AddNode(TSubclassOf<USpellNode> NodeClass);

    UFUNCTION(BlueprintCallable, Category = "Grimoire Editor")
    void ConnectNodes(USpellNode* Source, USpellNode* Target);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grimoire Editor")
    UPanelWidget* NodeContainer;

private:
    UPROPERTY()
    UHeartGraph* SpellGraph;
};