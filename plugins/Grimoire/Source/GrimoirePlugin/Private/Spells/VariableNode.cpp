#include "Spells/VariableNode.h"
#include "SpellExecutionContext.h"
#include "Components/GrimoireComponent.h"
#include "Engine/World.h"

UVariableNode::UVariableNode()
{
    NodeName = TEXT("Variable Node");
    NodeType = ESpellNodeType::Variable;
    NodeDescription = FText::FromString(TEXT("Stores and manipulates data values"));
    NodeIcon = FText::FromString(TEXT("📊"));
    NodeColor = FLinearColor::Purple;
    NodeCategory = FText::FromString(TEXT("Data"));
    
    VariableName = TEXT("MyVariable");
    VariableType = EGWTVariableType::Float;
    Operation = EVariableNodeOperation::Get;
    bIsGlobal = false;
    bPersistent = false;
    
    DefaultFloatValue = 0.0f;
    DefaultIntValue = 0;
    DefaultBoolValue = false;
    DefaultStringValue = TEXT("");
    DefaultVectorValue = FVector::ZeroVector;
    
    ClampMin = 0.0f;
    ClampMax = 100.0f;
    NodeManaCost = 3.0f;
}

TArray<FHeartGraphPinDesc> UVariableNode::GetInputPinDescs() const
{
    TArray<FHeartGraphPinDesc> Pins = Super::GetInputPinDescs();
    
    // Variable-specific input pins
    Pins.Add(CreateDataPin(TEXT("Value"), EHeartPinDirection::Input, TEXT("Float")));
    Pins.Add(CreateDataPin(TEXT("OperandB"), EHeartPinDirection::Input, TEXT("Float")));
    
    // Conditional pins based on operation
    if (Operation == EVariableNodeOperation::Set)
    {
        Pins.Add(CreateDataPin(TEXT("NewValue"), EHeartPinDirection::Input, TEXT("Struct")));
    }
    
    return Pins;
}

TArray<FHeartGraphPinDesc> UVariableNode::GetOutputPinDescs() const
{
    TArray<FHeartGraphPinDesc> Pins = Super::GetOutputPinDescs();
    
    // Variable-specific output pins
    Pins.Add(CreateDataPin(TEXT("CurrentValue"), EHeartPinDirection::Output, TEXT("Struct")));
    Pins.Add(CreateDataPin(TEXT("PreviousValue"), EHeartPinDirection::Output, TEXT("Struct")));
    Pins.Add(CreateDataPin(TEXT("Changed"), EHeartPinDirection::Output, TEXT("Bool")));
    
    // Rarity-based additional outputs
    if (NodeRarity >= EItemRarity::Rare)
    {
        Pins.Add(CreateDataPin(TEXT("ValueHistory"), EHeartPinDirection::Output, TEXT("Array")));
    }
    
    if (NodeRarity >= EItemRarity::Epic)
    {
        Pins.Add(CreateDataPin(TEXT("Statistics"), EHeartPinDirection::Output, TEXT("Struct")));
    }
    
    return Pins;
}

void UVariableNode::OnExecute(USpellExecutionContext* Context)
{
    Super::OnExecute(Context);
    
    if (!Context)
    {
        UE_LOG(LogTemp, Error, TEXT("VariableNode::OnExecute - Context is null"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VariableNode executing: %s (%s %s)"), 
        *NodeName, 
        *VariableName.ToString(),
        *UEnum::GetValueAsString(Operation));
    
    // Initialize variable if it doesn't exist
    if (!Context->HasVariable(VariableName, bIsGlobal))
    {
        InitializeVariable(Context);
    }
    
    // Load persistent value if needed
    if (bPersistent && NodeRarity == EItemRarity::Legendary)
    {
        LoadPersistentValue(Context);
    }
    
    // Get current value
    FGWTVariableValue CurrentValue = GetVariableValue(Context);
    FGWTVariableValue PreviousValue = CurrentValue; // Store for comparison
    
    // Perform the operation
    switch (Operation)
    {
        case EVariableNodeOperation::Get:
            // Just retrieve the value (no modification)
            break;
            
        case EVariableNodeOperation::Set:
        {
            // Set new value from input or default
            FGWTVariableValue NewValue = CreateDefaultValue();
            
            if (Context->HasVariable(TEXT("NewValue")))
            {
                NewValue = Context->GetVariable(TEXT("NewValue"));
            }
            else if (Context->HasVariable(TEXT("Value")))
            {
                FGWTVariableValue InputValue = Context->GetVariable(TEXT("Value"));
                NewValue = ConvertToType(InputValue, VariableType);
            }
            
            SetVariableValue(Context, NewValue);
            CurrentValue = NewValue;
            break;
        }
        
        default:
        {
            // Math operations
            FGWTVariableValue OperandB = CreateDefaultValue();
            
            if (Context->HasVariable(TEXT("OperandB")))
            {
                OperandB = Context->GetVariable(TEXT("OperandB"));
            }
            else if (Context->HasVariable(TEXT("Value")))
            {
                OperandB = Context->GetVariable(TEXT("Value"));
            }
            
            FGWTVariableValue Result = PerformMathOperation(CurrentValue, OperandB, Operation);
            SetVariableValue(Context, Result);
            CurrentValue = Result;
            break;
        }
    }
    
    // Check if value changed
    bool bValueChanged = (CurrentValue.Type != PreviousValue.Type) ||
                        (CurrentValue.FloatValue != PreviousValue.FloatValue) ||
                        (CurrentValue.IntValue != PreviousValue.IntValue) ||
                        (CurrentValue.BoolValue != PreviousValue.BoolValue) ||
                        (CurrentValue.StringValue != PreviousValue.StringValue) ||
                        (CurrentValue.VectorValue != PreviousValue.VectorValue);
    
    // Update history if value changed and rarity supports it
    if (bValueChanged && NodeRarity >= EItemRarity::Rare)
    {
        UpdateVariableHistory(Context, PreviousValue, CurrentValue);
    }
    
    // Save persistent value if needed
    if (bPersistent && NodeRarity == EItemRarity::Legendary && bValueChanged)
    {
        SavePersistentValue(CurrentValue);
    }
    
    // Set output variables
    Context->SetVariable(TEXT("CurrentValue"), CurrentValue);
    Context->SetVariable(TEXT("PreviousValue"), PreviousValue);
    Context->SetVariable(TEXT("ValueChanged"), FGWTVariableValue::FromBool(bValueChanged));
    
    // Apply rarity effects
    ApplyRarityEffects(Context);
    
    UE_LOG(LogTemp, Log, TEXT("Variable %s: %s -> %s (Changed: %s)"), 
        *VariableName.ToString(),
        *PreviousValue.StringValue,
        *CurrentValue.StringValue,
        bValueChanged ? TEXT("Yes") : TEXT("No"));
}

FGWTVariableValue UVariableNode::GetVariableValue(USpellExecutionContext* Context)
{
    if (Context->HasVariable(VariableName, bIsGlobal))
    {
        return Context->GetVariable(VariableName, bIsGlobal);
    }
    
    // Return default if variable doesn't exist
    return CreateDefaultValue();
}

void UVariableNode::SetVariableValue(USpellExecutionContext* Context, const FGWTVariableValue& Value)
{
    Context->SetVariable(VariableName, Value, bIsGlobal);
}

FGWTVariableValue UVariableNode::PerformMathOperation(const FGWTVariableValue& A, const FGWTVariableValue& B, EVariableNodeOperation Op)
{
    FGWTVariableValue Result = A;
    
    switch (Op)
    {
        case EVariableNodeOperation::Add:
            if (A.Type == EGWTVariableType::Float && B.Type == EGWTVariableType::Float)
            {
                Result.FloatValue = A.FloatValue + B.FloatValue;
            }
            else if (A.Type == EGWTVariableType::Int && B.Type == EGWTVariableType::Int)
            {
                Result.IntValue = A.IntValue + B.IntValue;
            }
            else if (A.Type == EGWTVariableType::Vector && B.Type == EGWTVariableType::Vector)
            {
                Result.VectorValue = A.VectorValue + B.VectorValue;
            }
            break;
            
        case EVariableNodeOperation::Subtract:
            if (A.Type == EGWTVariableType::Float && B.Type == EGWTVariableType::Float)
            {
                Result.FloatValue = A.FloatValue - B.FloatValue;
            }
            else if (A.Type == EGWTVariableType::Int && B.Type == EGWTVariableType::Int)
            {
                Result.IntValue = A.IntValue - B.IntValue;
            }
            else if (A.Type == EGWTVariableType::Vector && B.Type == EGWTVariableType::Vector)
            {
                Result.VectorValue = A.VectorValue - B.VectorValue;
            }
            break;
            
        case EVariableNodeOperation::Multiply:
            if (A.Type == EGWTVariableType::Float && B.Type == EGWTVariableType::Float)
            {
                Result.FloatValue = A.FloatValue * B.FloatValue;
            }
            else if (A.Type == EGWTVariableType::Int && B.Type == EGWTVariableType::Int)
            {
                Result.IntValue = A.IntValue * B.IntValue;
            }
            else if (A.Type == EGWTVariableType::Vector && B.Type == EGWTVariableType::Float)
            {
                Result.VectorValue = A.VectorValue * B.FloatValue;
            }
            break;
            
        case EVariableNodeOperation::Divide:
            if (A.Type == EGWTVariableType::Float && B.Type == EGWTVariableType::Float && B.FloatValue != 0.0f)
            {
                Result.FloatValue = A.FloatValue / B.FloatValue;
            }
            else if (A.Type == EGWTVariableType::Int && B.Type == EGWTVariableType::Int && B.IntValue != 0)
            {
                Result.IntValue = A.IntValue / B.IntValue;
            }
            break;
            
        case EVariableNodeOperation::Min:
            if (A.Type == EGWTVariableType::Float && B.Type == EGWTVariableType::Float)
            {
                Result.FloatValue = FMath::Min(A.FloatValue, B.FloatValue);
            }
            else if (A.Type == EGWTVariableType::Int && B.Type == EGWTVariableType::Int)
            {
                Result.IntValue = FMath::Min(A.IntValue, B.IntValue);
            }
            break;
            
        case EVariableNodeOperation::Max:
            if (A.Type == EGWTVariableType::Float && B.Type == EGWTVariableType::Float)
            {
                Result.FloatValue = FMath::Max(A.FloatValue, B.FloatValue);
            }
            else if (A.Type == EGWTVariableType::Int && B.Type == EGWTVariableType::Int)
            {
                Result.IntValue = FMath::Max(A.IntValue, B.IntValue);
            }
            break;
            
        case EVariableNodeOperation::Clamp:
            if (A.Type == EGWTVariableType::Float)
            {
                Result.FloatValue = FMath::Clamp(A.FloatValue, ClampMin, ClampMax);
            }
            else if (A.Type == EGWTVariableType::Int)
            {
                Result.IntValue = FMath::Clamp(A.IntValue, static_cast<int32>(ClampMin), static_cast<int32>(ClampMax));
            }
            break;
            
        case EVariableNodeOperation::Increment:
            if (A.Type == EGWTVariableType::Float)
            {
                Result.FloatValue = A.FloatValue + 1.0f;
            }
            else if (A.Type == EGWTVariableType::Int)
            {
                Result.IntValue = A.IntValue + 1;
            }
            break;
            
        case EVariableNodeOperation::Decrement:
            if (A.Type == EGWTVariableType::Float)
            {
                Result.FloatValue = A.FloatValue - 1.0f;
            }
            else if (A.Type == EGWTVariableType::Int)
            {
                Result.IntValue = A.IntValue - 1;
            }
            break;
    }
    
    return Result;
}

FGWTVariableValue UVariableNode::CreateDefaultValue()
{
    FGWTVariableValue DefaultValue;
    DefaultValue.Type = VariableType;
    
    switch (VariableType)
    {
        case EGWTVariableType::Float:
            DefaultValue.FloatValue = DefaultFloatValue;
            break;
        case EGWTVariableType::Int:
            DefaultValue.IntValue = DefaultIntValue;
            break;
        case EGWTVariableType::Bool:
            DefaultValue.BoolValue = DefaultBoolValue;
            break;
        case EGWTVariableType::String:
            DefaultValue.StringValue = DefaultStringValue;
            break;
        case EGWTVariableType::Vector:
            DefaultValue.VectorValue = DefaultVectorValue;
            break;
        default:
            break;
    }
    
    return DefaultValue;
}

void UVariableNode::InitializeVariable(USpellExecutionContext* Context)
{
    FGWTVariableValue InitialValue = CreateDefaultValue();
    Context->SetVariable(VariableName, InitialValue, bIsGlobal);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized variable %s with default value"), *VariableName.ToString());
}

FGWTVariableValue UVariableNode::ConvertToType(const FGWTVariableValue& Value, EGWTVariableType TargetType)
{
    FGWTVariableValue Result;
    Result.Type = TargetType;
    
    switch (TargetType)
    {
        case EGWTVariableType::Float:
            switch (Value.Type)
            {
                case EGWTVariableType::Int:
                    Result.FloatValue = static_cast<float>(Value.IntValue);
                    break;
                case EGWTVariableType::Bool:
                    Result.FloatValue = Value.BoolValue ? 1.0f : 0.0f;
                    break;
                case EGWTVariableType::String:
                    Result.FloatValue = FCString::Atof(*Value.StringValue);
                    break;
                default:
                    Result.FloatValue = Value.FloatValue;
                    break;
            }
            break;
            
        case EGWTVariableType::Int:
            switch (Value.Type)
            {
                case EGWTVariableType::Float:
                    Result.IntValue = static_cast<int32>(Value.FloatValue);
                    break;
                case EGWTVariableType::Bool:
                    Result.IntValue = Value.BoolValue ? 1 : 0;
                    break;
                case EGWTVariableType::String:
                    Result.IntValue = FCString::Atoi(*Value.StringValue);
                    break;
                default:
                    Result.IntValue = Value.IntValue;
                    break;
            }
            break;
            
        case EGWTVariableType::Bool:
            switch (Value.Type)
            {
                case EGWTVariableType::Float:
                    Result.BoolValue = Value.FloatValue != 0.0f;
                    break;
                case EGWTVariableType::Int:
                    Result.BoolValue = Value.IntValue != 0;
                    break;
                case EGWTVariableType::String:
                    Result.BoolValue = Value.StringValue.Len() > 0;
                    break;
                default:
                    Result.BoolValue = Value.BoolValue;
                    break;
            }
            break;
            
        case EGWTVariableType::String:
            switch (Value.Type)
            {
                case EGWTVariableType::Float:
                    Result.StringValue = FString::Printf(TEXT("%.2f"), Value.FloatValue);
                    break;
                case EGWTVariableType::Int:
                    Result.StringValue = FString::Printf(TEXT("%d"), Value.IntValue);
                    break;
                case EGWTVariableType::Bool:
                    Result.StringValue = Value.BoolValue ? TEXT("true") : TEXT("false");
                    break;
                case EGWTVariableType::Vector:
                    Result.StringValue = Value.VectorValue.ToString();
                    break;
                default:
                    Result.StringValue = Value.StringValue;
                    break;
            }
            break;
            
        default:
            Result = Value;
            break;
    }
    
    return Result;
}

void UVariableNode::UpdateVariableHistory(USpellExecutionContext* Context, const FGWTVariableValue& OldValue, const FGWTVariableValue& NewValue)
{
    if (!VariableHistories.Contains(VariableName))
    {
        VariableHistories.Add(VariableName, FVariableHistory());
    }
    
    FVariableHistory& History = VariableHistories[VariableName];
    
    UWorld* World = GetWorld();
    float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
    
    History.AddValue(OldValue, CurrentTime);
    
    // Set history data in context for other nodes to use
    Context->SetVariable(TEXT("VariableHistory"), FGWTVariableValue::FromString(TEXT("Available")));
    
    UE_LOG(LogTemp, Log, TEXT("Updated history for variable %s (entries: %d)"), 
        *VariableName.ToString(), History.PreviousValues.Num());
}

void UVariableNode::LoadPersistentValue(USpellExecutionContext* Context)
{
    if (PersistentValues.Contains(VariableName))
    {
        FGWTVariableValue PersistentValue = PersistentValues[VariableName];
        Context->SetVariable(VariableName, PersistentValue, bIsGlobal);
        
        UE_LOG(LogTemp, Log, TEXT("Loaded persistent value for variable %s"), *VariableName.ToString());
    }
}

void UVariableNode::SavePersistentValue(const FGWTVariableValue& Value)
{
    PersistentValues.Add(VariableName, Value);
    
    UE_LOG(LogTemp, Log, TEXT("Saved persistent value for variable %s"), *VariableName.ToString());
}

void UVariableNode::ApplyRarityEffects(USpellExecutionContext* Context)
{
    switch (NodeRarity)
    {
        case EItemRarity::Uncommon:
            // Automatic type conversion support
            Context->SetVariable(TEXT("AutoConvert"), FGWTVariableValue::FromBool(true));
            break;
            
        case EItemRarity::Rare:
            // History tracking is already implemented
            if (VariableHistories.Contains(VariableName))
            {
                const FVariableHistory& History = VariableHistories[VariableName];
                Context->SetVariable(TEXT("HistoryCount"), FGWTVariableValue::FromInt(History.PreviousValues.Num()));
            }
            break;
            
        case EItemRarity::Epic:
            // Statistical analysis
            if (VariableHistories.Contains(VariableName))
            {
                const FVariableHistory& History = VariableHistories[VariableName];
                if (History.PreviousValues.Num() > 1)
                {
                    // Calculate simple statistics for float values
                    float Sum = 0.0f;
                    int32 Count = 0;
                    
                    for (const FGWTVariableValue& Val : History.PreviousValues)
                    {
                        if (Val.Type == EGWTVariableType::Float)
                        {
                            Sum += Val.FloatValue;
                            Count++;
                        }
                    }
                    
                    if (Count > 0)
                    {
                        float Average = Sum / Count;
                        Context->SetVariable(TEXT("VariableAverage"), FGWTVariableValue::FromFloat(Average));
                    }
                }
            }
            break;
            
        case EItemRarity::Legendary:
            // Persistent memory is already implemented
            Context->SetVariable(TEXT("HasPersistentMemory"), FGWTVariableValue::FromBool(true));
            break;
    }
}

float UVariableNode::GetBasePower() const
{
    return 0.0f; // Variables don't provide power directly
}