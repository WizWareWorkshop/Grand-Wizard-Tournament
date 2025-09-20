// Source/GrimoirePlugin/Private/TutorialManager.cpp
#include "TutorialManager.h"
#include "Kismet/KismetSystemLibrary.h"

void UTutorialManager::ShowNextHint()
{
    if (TutorialSteps.Num() > 0)
    {
        FTutorialStep Step = TutorialSteps[0];
        UKismetSystemLibrary::PrintString(GetWorld(), Step.HintText.ToString(), true, false, FLinearColor::Yellow, 5.0f);
    }
}

bool UTutorialManager::CheckStepCompletion(USpellNode* AddedNode)
{
    if (TutorialSteps.Num() > 0 && AddedNode && TutorialSteps[0].RequiredNode == AddedNode->GetClass())
    {
        TutorialSteps.RemoveAt(0);
        ShowNextHint();
        return true;
    }
    return false;
}