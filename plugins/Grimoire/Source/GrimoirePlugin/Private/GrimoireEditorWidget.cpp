#include "GrimoireEditorWidget.h"
#include "Model/HeartGraph.h"
#include "SpellNode.h"
#include "Components/PanelWidget.h"

void UGrimoireEditorWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (!SpellGraph)
    {
        SpellGraph = NewObject<UHeartGraph>(this);
    }
}

void UGrimoireEditorWidget::AddNode(TSubclassOf<USpellNode> NodeClass)
{
    if (SpellGraph && NodeClass)
    {
        USpellNode* NewNode = NewObject<USpellNode>(SpellGraph, NodeClass);
        SpellGraph->AddNode(NewNode);
        
        // Set default position
        NewNode->Position = FVector2D(100, 100);
    }
}

void UGrimoireEditorWidget::ConnectNodes(USpellNode* Source, USpellNode* Target)
{
    if (SpellGraph && Source && Target)
    {
        // Basic connection - will need to implement proper pin connections
        // This is a placeholder for the actual HeartGraph connection logic
        UE_LOG(LogTemp, Warning, TEXT("Connecting nodes: %s to %s"), 
            *Source->GetName(), *Target->GetName());
    }
}