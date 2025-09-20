// Source/GrimoirePlugin/Private/TriggerNode.cpp
#include "TriggerNode.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UTriggerNode::Execute(UObject* Context)
{
    Super::Execute(Context); // Validate and propagate

    UWorld* World = Context ? Context->GetWorld() : nullptr;
    if (!World) return;

    switch (EventType)
    {
        case ETriggerEventType::OnCast:
            // Immediate execution
            break;
        case ETriggerEventType::OnHit:
            // Assume collision detection from parent spell; simulate for test
            UE_LOG(LogTemp, Log, TEXT("Trigger: OnHit Detected"));
            break;
        case ETriggerEventType::OnEnemyEnter:
            // Use overlap events (assume setup in spell actor)
            if (TriggerRange > 0.0f)
            {
                // Simulate range check
                UE_LOG(LogTemp, Log, TEXT("Trigger: Enemy Entered Range %.2f"), TriggerRange);
            }
            break;
        case ETriggerEventType::OnTimer:
            if (TimerInterval > 0.0f)
            {
                World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &UTriggerNode::HandleTimerTrigger, Context), TimerInterval, true);
            }
            break;
        default:
            break;
    }
}

float UTriggerNode::GetBasePower() const
{
    return 0.0f; // Triggers don't have power; modifier only
}

void UTriggerNode::HandleTimerTrigger(UObject* Context)
{
    UE_LOG(LogTemp, Log, TEXT("Trigger: Timer Fired"));
    Super::Execute(Context); // Re-propagate on timer
}