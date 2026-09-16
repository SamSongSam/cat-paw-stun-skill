#pragma once
// #region SUMMARY
// File: EconomyComponent.h
// Purpose: Generic gold wallet (section 11) — deliberately knows nothing
//          about skills. CatSkillComponent never touches CurrentGold
//          directly; it sends a GoldCost effect and EffectReceiverComponent
//          calls SpendGold() here.
// Depends on: none
// Consumed by: UEffectReceiverComponent (GoldCost effect -> SpendGold)
// Exposed params: CurrentGold/MaxGold — EditAnywhere so a designer can seed
//                 starting gold per-enemy in the editor
// #endregion

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EconomyComponent.generated.h"

UCLASS(ClassGroup = (CatSkill), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UEconomyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEconomyComponent();

	UFUNCTION(BlueprintPure, Category = "Economy")
	bool CanAfford(int32 Amount) const { return CurrentGold >= Amount; }

	// Prototype policy per section 11: "Gold > 0 -> Spend Gold" — fails
	// (returns false, no partial spend) if CanAfford() is false. Add a
	// fallback/debt policy later only if a skill actually needs one.
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool SpendGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Economy")
	int32 GetCurrentGold() const { return CurrentGold; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy", meta = (ClampMin = "0"))
	int32 CurrentGold = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy", meta = (ClampMin = "0"))
	int32 MaxGold = 999;

	UFUNCTION(BlueprintImplementableEvent, Category = "Economy")
	void OnGoldSpent(int32 Amount, int32 NewTotal);
};
