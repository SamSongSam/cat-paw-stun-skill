#pragma once
// #region SUMMARY
// File: StatusComponent.h
// Purpose: Applies/removes timed status effects on the owning actor (section
//          10). Phase 6 scope: Stun only — Slow/Burn/Freeze/Silence/Knockdown
//          are named in the doc but NOT implemented; add them only when a
//          skill actually needs one, following this same ApplyX/timer pattern.
// Depends on: none beyond engine timer manager
// Consumed by: UEffectReceiverComponent (Stun effect -> ApplyStun)
// Exposed params: none yet — Stun duration always comes from the effect spec
//                 that triggered it, never hardcoded here
// #endregion

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"

UCLASS(ClassGroup = (CatSkill), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusComponent();

	// No-ops (does not restart the timer) if already stunned — see section
	// 10 flow: stop movement, disable AI, play stunned state, timer, remove.
	UFUNCTION(BlueprintCallable, Category = "Status")
	void ApplyStun(float Duration);

	UFUNCTION(BlueprintPure, Category = "Status")
	bool IsStunned() const { return bIsStunned; }

protected:
	void RemoveStun();

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bIsStunned = false;

	FTimerHandle StunTimerHandle;

	// Presentation hooks — Blueprint/AnimBP reacts to these (Golden Rule:
	// C++ owns the state, Blueprint owns how it looks).
	UFUNCTION(BlueprintImplementableEvent, Category = "Status")
	void OnStunStarted(float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status")
	void OnStunEnded();
};
