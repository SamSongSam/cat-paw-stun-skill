#pragma once
// #region SUMMARY
// File: CatSkillComponent.h
// Purpose: Phase 1-2 slice of the Cat Paw Stun Skill (see CatSkill_test.md section 5). Owns
//          cooldown/state and spawns the aura Niagara system on activation. Target detection,
//          projectile spawn, and gameplay effects (stun/gold/carry) are NOT here yet — added in
//          later phases per the doc's own vertical-slice build order (section 35).
// Depends on: UCatSkillData (CatSkillData.h) for tunable values; UNiagaraSystem (engine)
// Consumed by: ACatPlayerCharacter (calls ActivateCatSkill() from Enhanced Input); Blueprint
//              subclasses of the owning character (bind OnCatSkillActivated for presentation)
// Exposed params: SkillData (EditAnywhere on the owning actor) — everything else the skill needs
//                 (cooldown, aura system, attach socket) lives on the Data Asset it points to
// #endregion

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "CatSkillComponent.generated.h"

class UCatSkillData;
class UNiagaraComponent;
class ACatPawProjectile;

/**
 * Runtime driver for one cat skill instance. C++ owns state/validation/spawning (gameplay);
 * Blueprint listens to OnCatSkillActivated/OnSkillFinished to add animation, sound, camera work
 * (presentation) — see the Golden Rule in CatSkill_test.md section 40.
 */
UCLASS(ClassGroup = (CatSkill), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UCatSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCatSkillComponent();

	// #region Public API
	// Entry point wired from Enhanced Input (see ACatPlayerCharacter::HandleCatSkillInput). Runs
	// the whole cast in one call: cooldown check -> target search -> aura -> projectile.
	UFUNCTION(BlueprintCallable, Category = "Cat Skill")
	void ActivateCatSkill();

	// True only when SkillData is assigned, no cast is currently in flight, and cooldown has
	// elapsed since LastCastTime. Safe to call every frame from Blueprint (e.g. to grey out a UI icon).
	UFUNCTION(BlueprintPure, Category = "Cat Skill")
	bool CanActivateSkill() const;
	// #endregion

protected:
	virtual void BeginPlay() override;

	// #region Cast pipeline (called in this order from ActivateCatSkill)
	// Phase 2: spawns SkillData->AuraSystem attached to the owner. No-op (with a log warning) if
	// SkillData or AuraSystem is unset — never assume the designer has assigned them (see UE
	// crash-prevention checklist: always IsValid() a UPROPERTY asset reference before use).
	// TargetActor (may be null — no target found yet) is used only to read its current
	// StunStack/FXIntensity so the aura reflects gameplay state on spawn (NEXT.md section 7);
	// it does not change what the aura attaches to or how it's spawned.
	void SpawnAura(AActor* TargetActor);

	// Phase 3: sphere overlap within SkillData->Range, filtered to actors that
	// have an EffectReceiverComponent (section 29 — never a specific enemy
	// class), then picks the closest one (section 30). Returns nullptr if nothing qualifies —
	// callers must IsValid()-check before using the result (see ActivateCatSkill).
	AActor* FindTarget() const;

	// Phase 4: spawns SkillData->ProjectileClass and hands it TargetActor +
	// SkillData->Effects. No-op (with a warning) if ProjectileClass is unset.
	void SpawnProjectile(AActor* TargetActor);

	// Phase 6-8: builds one FGameplayEffectSpec per entry in SkillData->Effects
	// (Source/Target filled in here) and forwards it to the projectile — the
	// projectile applies them on hit, this component never touches the
	// target's components directly (Golden Rule: skill doesn't know enemy).
	TArray<FGameplayEffectSpec> BuildEffectSpecs(AActor* TargetActor) const;
	// #endregion

	// #region Tunables and runtime state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Skill")
	TObjectPtr<UCatSkillData> SkillData;

	// Section 36 — Debug Mode. Draws SkillData->Range as a sphere and the
	// found target (if any) on every ActivateCatSkill call. Never affects
	// gameplay logic, only DrawDebug* calls.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Skill|Debug")
	bool bDebugSkill = false;

	// True only while ActivateCatSkill's own call stack is executing — this is not a "skill is on
	// cooldown" flag (see LastCastTime for that), it exists purely to reject re-entrant activation.
	UPROPERTY(BlueprintReadOnly, Category = "Cat Skill")
	bool bIsSkillActive = false;

	// -1.0f sentinel = "never cast yet"; see CanActivateSkill()'s cooldown formula and
	// INTEGRATION_NOTES.md section 4 for why the sentinel check is split out from the subtraction.
	UPROPERTY(BlueprintReadOnly, Category = "Cat Skill")
	float LastCastTime = -1.0f;

	// Tracks the spawned aura so a future phase can stop/fade it explicitly instead of leaking it.
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActiveAuraComponent;
	// #endregion

	// #region Presentation hooks (Blueprint plays animation/sound/camera work here — C++ never does)
	UFUNCTION(BlueprintImplementableEvent, Category = "Cat Skill")
	void OnCatSkillActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cat Skill")
	void OnTargetAcquired(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cat Skill")
	void OnProjectileSpawned(ACatPawProjectile* Projectile);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cat Skill")
	void OnSkillFinished();
	// #endregion
};
