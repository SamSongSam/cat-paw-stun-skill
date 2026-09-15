// #region SUMMARY
// File: CatSkillComponent.cpp
// Purpose: Implements Phase 1-2 activation flow — cooldown/state check, spawn aura attached to
//          owner, notify Blueprint. See CatSkillComponent.h for scope boundary.
// Depends on: CatSkillData.h, NiagaraFunctionLibrary (engine)
// Consumed by: ACatPlayerCharacter
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "CatSkillComponent.h"
#include "CatSkillData.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UCatSkillComponent::UCatSkillComponent()
{
	// This component only needs to react to explicit calls (ActivateCatSkill) — no per-frame work.
	PrimaryComponentTick.bCanEverTick = false;
}

void UCatSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!SkillData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] %s has no SkillData assigned — skill will no-op."),
			*GetOwner()->GetName());
	}
}

bool UCatSkillComponent::CanActivateSkill() const
{
	if (!IsValid(SkillData))
	{
		return false;
	}

	if (bIsSkillActive)
	{
		return false;
	}

	if (LastCastTime < 0.0f)
	{
		// Never cast yet this session — cooldown cannot block the first cast.
		return true;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	return (World->GetTimeSeconds() - LastCastTime) >= SkillData->Cooldown;
}

void UCatSkillComponent::ActivateCatSkill()
{
	if (!CanActivateSkill())
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	bIsSkillActive = true;
	LastCastTime = World->GetTimeSeconds();

	SpawnAura();

	OnCatSkillActivated();

	// Phase 1-2 scope ends here: no target detection / projectile / effects yet, so the skill
	// instance is considered "finished" the moment the aura is up. Later phases will move this
	// call to after the full sequence (hit -> stun -> gold -> deliver -> lick) completes.
	bIsSkillActive = false;
	OnSkillFinished();
}

void UCatSkillComponent::SpawnAura()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	if (!IsValid(SkillData) || !IsValid(SkillData->AuraSystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] SpawnAura skipped — SkillData or AuraSystem missing on %s."),
			*Owner->GetName());
		return;
	}

	USceneComponent* AttachTarget = Owner->GetRootComponent();
	if (!IsValid(AttachTarget))
	{
		return;
	}

	ActiveAuraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SkillData->AuraSystem,
		AttachTarget,
		SkillData->AuraAttachSocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		/*bAutoDestroy=*/true);
}
