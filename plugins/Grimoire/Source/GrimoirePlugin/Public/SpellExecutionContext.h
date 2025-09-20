// Source/GrimoirePlugin/Public/SpellExecutionContext.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GrimoireTypes.h"
#include "SpellExecutionContext.generated.h"

USTRUCT(BlueprintType)
struct FGWTVariableValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGWTVariableType Type = EGWTVariableType::None;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloatValue = 0.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 IntValue = 0;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool BoolValue = false;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StringValue;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VectorValue;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* ObjectValue = nullptr;

	// Conversion helpers
	static FGWTVariableValue FromFloat(float Value);
	static FGWTVariableValue FromInt(int32 Value);
	static FGWTVariableValue FromBool(bool Value);
	static FGWTVariableValue FromString(const FString& Value);
	static FGWTVariableValue FromVector(const FVector& Value);
	static FGWTVariableValue FromActor(AActor* Value);
};

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Spell Execution Context"))
class GRIMOIREPLUGIN_API UGWTSpellExecutionContext : public UObject
{
	GENERATED_BODY()

public:
	// Core context data
	UPROPERTY(BlueprintReadWrite, Category = "Spell")
	AActor* Caster = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Spell")
	AActor* Target = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Spell")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Spell")
	FHitResult HitResult;

	// Variable storage with scoping support
	UPROPERTY(BlueprintReadOnly, Category = "Spell")
	TMap<FName, FGWTVariableValue> LocalVariables;

	UPROPERTY(BlueprintReadOnly, Category = "Spell")
	TMap<FName, FGWTVariableValue> GlobalVariables;

	// Execution state
	UPROPERTY(BlueprintReadOnly, Category = "Spell")
	float ExecutionTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Spell")
	int32 ExecutionDepth = 0;

	// API
	UFUNCTION(BlueprintCallable, Category = "Spell|Variables")
	void SetVariable(FName Name, const FGWTVariableValue& Value, bool bGlobal = false);

	UFUNCTION(BlueprintCallable, Category = "Spell|Variables")
	FGWTVariableValue GetVariable(FName Name, bool bGlobal = false) const;

	UFUNCTION(BlueprintCallable, Category = "Spell|Variables")
	bool HasVariable(FName Name, bool bGlobal = false) const;

	UFUNCTION(BlueprintCallable, Category = "Spell|Variables")
	void ClearVariables(bool bGlobal = false);

	// Execution control
	UFUNCTION(BlueprintCallable, Category = "Spell|Execution")
	USpellExecutionContext* CreateChildContext() const;

	UFUNCTION(BlueprintCallable, Category = "Spell|Execution")
	void MergeChildContext(const USpellExecutionContext* ChildContext);

	// Debugging
	UFUNCTION(BlueprintCallable, Category = "Spell|Debug")
	FString GetDebugString() const;
};