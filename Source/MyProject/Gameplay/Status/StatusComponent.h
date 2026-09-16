#pragma once
// #region SUMMARY
// File: StatusComponent.h
// Purpose: Applies/removes timed status effects on the owning actor (section
//          10). Phase 6 scope: Stun only — Slow/Burn/Freeze/Silence/Knockdown
//          are named in the doc but NOT implemented; add them only when a
//          skill actually needs one, following this same ApplyX/timer pattern.
//          Also owns StunStack (NEXT.md section 3) — the 0-4 "how many times has
//          this actor been zapped" counter that drives Dynamic FX escalation. It
//          lives here (not in CatSkillComponent) because the target, not the
//          caster, is the thing the escalation is about, and StatusComponent is
//          already the existing per-actor status authority — no second stack.
// Depends on: none beyond engine timer manager
// Consumed by: UEffectReceiverComponent (Stun effect -> ApplyStun); CatSkillComponent
//              and CatPawProjectile read GetStunStack() to drive Niagara User Parameters
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

	// NEXT.md section 3: gameplay-driven stack, max 4, incremented once per ApplyStun call
	// (including re-hits while already stunned — that's what "repeated hits escalate" means).
	// Clamps at MaxStunStack and stays there; nothing resets it automatically yet, since the
	// Cat Treat/Lick payoff that owns the reset trigger is a later phase (NEXT.md section 11) —
	// wiring a reset to nothing would be inventing behavior that isn't specified.
	UFUNCTION(BlueprintPure, Category = "Status")
	int32 GetStunStack() const { return StunStack; }

	// For the future payoff phase to call once it exists — not invoked anywhere yet.
	UFUNCTION(BlueprintCallable, Category = "Status")
	void ResetStunStack();

	static constexpr int32 MaxStunStack = 4;

protected:
	void RemoveStun();

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bIsStunned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	int32 StunStack = 0;

	FTimerHandle StunTimerHandle;

	// Presentation hooks — Blueprint/AnimBP reacts to these (Golden Rule:
	// C++ owns the state, Blueprint owns how it looks).
	UFUNCTION(BlueprintImplementableEvent, Category = "Status")
	void OnStunStarted(float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status")
	void OnStunEnded();

	// Fires every time StunStack changes — the FX layer's hook for reacting immediately rather
	// than polling GetStunStack(). Also fires once, edge-triggered, when the stack first hits
	// MaxStunStack (OnStunStackMaxed), so a future payoff phase has a clean trigger to bind to.
	UFUNCTION(BlueprintImplementableEvent, Category = "Status")
	void OnStunStackChanged(int32 NewStack);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status")
	void OnStunStackMaxed();
};
