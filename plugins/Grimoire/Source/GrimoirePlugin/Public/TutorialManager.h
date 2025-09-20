// Source/GrimoirePlugin/Public/TutorialManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TutorialManager.generated.h"

USTRUCT(BlueprintType)
struct FTutorialStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText HintText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<USpellNode> RequiredNode;
};

UCLASS(Blueprintable)
class GRIMOIREPLUGIN_API UTutorialManager : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FTutorialStep> TutorialSteps;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ShowNextHint();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool CheckStepCompletion(USpellNode* AddedNode);
};