#pragma once
// #region SUMMARY
// File: StatusComponent.h
// Purpose: Applies/removes timed status effects on the owning actor (section
//          10). Phase 6 scope: Stun only — Slow/Burn/Freeze/Silence/Knockdown
//          are named in the doc but NOT implemented; add them only when a
//          skill actually needs one, following this same ApplyX/timer pattern.
//          Also owns StunStack (NEXT.md section 3) — the 0-4 "how many times has
//          this actor been zapped" counter that drives Dynamic FX escalation AND
//          gold-cost scaling (CatSkillComponent::BuildEffectSpecs). It lives here
//          (not in CatSkillComponent) because the target, not the caster, is the
//          thing the escalation is about, and StatusComponent is already the
//          existing per-actor status authority — no second stack. Cycles
//          0->1->2->3->4->reset automatically: RemoveStun() resets it once the
//          stun that pushed it to MaxStunStack naturally wears off.
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

	// #region Stun API
	// No-ops (does not restart the timer) if already stunned — see section
	// 10 flow: stop movement, disable AI, play stunned state, timer, remove.
	UFUNCTION(BlueprintCallable, Category = "Status")
	void ApplyStun(float Duration);

	// True for the duration passed to the most recent ApplyStun() call, false once RemoveStun()'s
	// timer fires. Read this instead of touching bIsStunned directly from outside the class.
	UFUNCTION(BlueprintPure, Category = "Status")
	bool IsStunned() const { return bIsStunned; }
	// #endregion

	// #region StunStack API (NEXT.md section 3 — Dynamic FX escalation counter)
	// Current escalation count, 0..MaxStunStack. Incremented once per ApplyStun call (including
	// re-hits while already stunned — that's what "repeated hits escalate" means). Read this from
	// the FX layer (CatSkillComponent/CatPawProjectile) to drive Niagara User Parameters.
	UFUNCTION(BlueprintPure, Category = "Status")
	int32 GetStunStack() const { return StunStack; }

	// Escalation ceiling for THIS actor instance (see MaxStunStack below for why it's per-instance).
	UFUNCTION(BlueprintPure, Category = "Status")
	int32 GetMaxStunStack() const { return MaxStunStack; }

	// Zeroes the stack and broadcasts OnStunStackChanged. Called automatically by RemoveStun() once
	// a stun that maxed the stack wears off (completing the 0->4->reset cycle) — also
	// BlueprintCallable so a future Cat Treat/Lick payoff phase (NEXT.md section 11) can trigger an
	// early reset of its own once that system exists; nothing else calls it yet.
	UFUNCTION(BlueprintCallable, Category = "Status")
	void ResetStunStack();
	// #endregion

protected:
	// #region Internal helpers
	// Timer callback bound in ApplyStun; restores movement and fires OnStunEnded.
	void RemoveStun();
	// #endregion

	// #region State (Golden Rule: C++ owns gameplay state, Blueprint only reacts to it)
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bIsStunned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	int32 StunStack = 0;

	// Was a hardcoded `static constexpr int32 = 4` — moved to EditDefaultsOnly per this project's
	// own code-style rule 3 (every tunable value auto-exposes to the editor). A designer can now
	// give a specific enemy (e.g. a boss) a different escalation cap without a C++ recompile. It's
	// a per-instance UPROPERTY rather than a shared constant precisely so different enemy Blueprints
	// can set different values — that's also why callers outside this class (CatSkillComponent,
	// CatPawProjectile) must go through GetMaxStunStack() on a specific component instance instead
	// of reading a static value.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status", meta = (ClampMin = "1"))
	int32 MaxStunStack = 4;

	// Drives RemoveStun() — not exposed, purely an implementation detail of the timed-stun mechanic.
	FTimerHandle StunTimerHandle;
	// #endregion

	// #region Presentation hooks (Blueprint/AnimBP reacts to these — Golden Rule: C++ owns state,
	// Blueprint owns how it looks; none of these are implemented in C++, they're pure hand-off points)
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
	// #endregion
};
