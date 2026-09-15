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
#include "CatSkillComponent.generated.h"

class UCatSkillData;
class UNiagaraComponent;

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

	// Entry point wired from Enhanced Input (see ACatPlayerCharacter::HandleCatSkillInput).
	UFUNCTION(BlueprintCallable, Category = "Cat Skill")
	void ActivateCatSkill();

	UFUNCTION(BlueprintPure, Category = "Cat Skill")
	bool CanActivateSkill() const;

protected:
	virtual void BeginPlay() override;

	// Phase 2: spawns SkillData->AuraSystem attached to the owner. No-op (with a log warning) if
	// SkillData or AuraSystem is unset — never assume the designer has assigned them (see UE
	// crash-prevention checklist: always IsValid() a UPROPERTY asset reference before use).
	void SpawnAura();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Skill")
	TObjectPtr<UCatSkillData> SkillData;

	UPROPERTY(BlueprintReadOnly, Category = "Cat Skill")
	bool bIsSkillActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Cat Skill")
	float LastCastTime = -1.0f;

	// Tracks the spawned aura so a future phase can stop/fade it explicitly instead of leaking it.
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActiveAuraComponent;

	// Blueprint hooks for presentation (animation, sound, camera) — C++ never plays those itself.
	UFUNCTION(BlueprintImplementableEvent, Category = "Cat Skill")
	void OnCatSkillActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cat Skill")
	void OnSkillFinished();
};
