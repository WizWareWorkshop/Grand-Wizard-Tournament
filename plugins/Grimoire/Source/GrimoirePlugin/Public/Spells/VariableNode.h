#pragma once

#include "CoreMinimal.h"
#include "Spells/SpellNode.h"
#include "GrimoireTypes.h"
#include "VariableNode.generated.h"

UENUM(BlueprintType)
enum class EVariableNodeOperation : uint8
{
    Get         UMETA(DisplayName = "Get Value"),
    Set         UMETA(DisplayName = "Set Value"),
    Add         UMETA(DisplayName = "Add"),
    Subtract    UMETA(DisplayName = "Subtract"),
    Multiply    UMETA(DisplayName = "Multiply"),
    Divide      UMETA(DisplayName = "Divide"),
    Min         UMETA(DisplayName = "Minimum"),
    Max         UMETA(DisplayName = "Maximum"),
    Clamp       UMETA(DisplayName = "Clamp"),
    Increment   UMETA(DisplayName = "Increment"),
    Decrement   UMETA(DisplayName = "Decrement"),
    MAX         UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FVariableHistory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGWTVariableValue> PreviousValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> Timestamps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHistorySize = 10;

    void AddValue(const FGWTVariableValue& Value, float Timestamp)
    {
        PreviousValues.Add(Value);
        Timestamps.Add(Timestamp);

        // Keep only the most recent entries
        while (PreviousValues.Num() > MaxHistorySize)
        {
            PreviousValues.RemoveAt(0);
            Timestamps.RemoveAt(0);
        }
    }

    FGWTVariableValue GetLastValue() const
    {
        return PreviousValues.Num() > 0 ? PreviousValues.Last() : FGWTVariableValue();
    }
};

UCLASS(Blueprintable, meta = (DisplayName = "Variable Node"))
class GRIMOIREPLUGIN_API UVariableNode : public USpellNode
{
    GENERATED_BODY()

public:
    UVariableNode();

    // Variable properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    FName VariableName = TEXT("MyVariable");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    EGWTVariableType VariableType = EGWTVariableType::Float;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    EVariableNodeOperation Operation = EVariableNodeOperation::Get;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    bool bIsGlobal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable")
    bool bPersistent = false;

    // Default values
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Defaults")
    float DefaultFloatValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Defaults")
    int32 DefaultIntValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Defaults")
    bool DefaultBoolValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Defaults")
    FString DefaultStringValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Defaults")
    FVector DefaultVectorValue = FVector::ZeroVector;

    // Math operation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Math", meta = (EditCondition = "Operation == EVariableNodeOperation::Clamp"))
    float ClampMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Math", meta = (EditCondition = "Operation == EVariableNodeOperation::Clamp"))
    float ClampMax = 100.0f;

    // Execution
    virtual void OnExecute(USpellExecutionContext* Context) override;
    virtual float GetBasePower() const override;

    // Pin system override for variable-specific pins
    virtual TArray<FHeartGraphPinDesc> GetInputPinDescs() const override;
    virtual TArray<FHeartGraphPinDesc> GetOutputPinDescs() const override;

    // Variable operations
    UFUNCTION(BlueprintCallable, Category = "Variable")
    FGWTVariableValue GetVariableValue(USpellExecutionContext* Context);

    UFUNCTION(BlueprintCallable, Category = "Variable")
    void SetVariableValue(USpellExecutionContext* Context, const FGWTVariableValue& Value);

    UFUNCTION(BlueprintCallable, Category = "Variable")
    FGWTVariableValue PerformMathOperation(const FGWTVariableValue& A, const FGWTVariableValue& B, EVariableNodeOperation Op);

protected:
    // Variable management
    FGWTVariableValue CreateDefaultValue();
    void InitializeVariable(USpellExecutionContext* Context);
    
    // Type conversion
    FGWTVariableValue ConvertToType(const FGWTVariableValue& Value, EGWTVariableType TargetType);
    
    // History tracking (for Rare+ rarity)
    void UpdateVariableHistory(USpellExecutionContext* Context, const FGWTVariableValue& OldValue, const FGWTVariableValue& NewValue);
    
    // Persistence (for Legendary rarity)
    void LoadPersistentValue(USpellExecutionContext* Context);
    void SavePersistentValue(const FGWTVariableValue& Value);

    // Rarity effects
    void ApplyRarityEffects(USpellExecutionContext* Context);

private:
    // History storage
    UPROPERTY()
    TMap<FName, FVariableHistory> VariableHistories;

    // Persistent storage (simulated)
    UPROPERTY()
    TMap<FName, FGWTVariableValue> PersistentValues;
};