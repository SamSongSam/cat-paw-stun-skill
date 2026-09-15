#pragma once
// #region SUMMARY
// File: CatSkillData.h
// Purpose: Data-driven definition of one cat skill (Phase 1-2 scope: name, cooldown, aura VFX).
//          Add fields here — not hardcoded in CatSkillComponent — so new skills (DA_FireSkill,
//          DA_IceSkill, ...) can be authored as new Data Asset instances without touching C++.
// Depends on: none (leaf data class)
// Consumed by: UCatSkillComponent (reads SkillData at runtime)
// Exposed params: SkillDisplayName, Cooldown, AuraSystem, AuraAttachSocketName — all editable on
//                 the Data Asset instance in the Content Browser (no code change to tune a skill)
// #endregion

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CatSkillData.generated.h"

class UNiagaraSystem;

/**
 * Data-only description of a cat skill. Deliberately holds only what Phase 1-2 (input -> aura)
 * needs; extend with Range/ProjectileClass/Effects[] when the later phases in CatSkill_test.md
 * (projectile, stun, economy, ...) are implemented — do not pre-add unused fields now.
 */
UCLASS(BlueprintType)
class MYPROJECT_API UCatSkillData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Auto-exposed to the Data Asset editor so designers can rename/re-tune without a recompile.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Info")
	FText SkillDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float Cooldown = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|VFX")
	TObjectPtr<UNiagaraSystem> AuraSystem;

	// Socket on the owning mesh the aura attaches to; NAME_None attaches to the component root.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|VFX")
	FName AuraAttachSocketName = NAME_None;
};
