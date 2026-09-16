#pragma once
// #region SUMMARY
// File: AnimNotify_Lick.h
// Purpose: Phase 11 timing marker (section 20) — placed on AM_Cat_LickTreat
//          at the lick frame. Pure timing signal for Blueprint's Niagara
//          sparkle + sound (Golden Rule: FX/Presentation -> Niagara/Blueprint).
// Depends on: none beyond UAnimNotify
// Consumed by: AM_Cat_LickTreat (Animation Montage, authored in-editor)
// Exposed params: none — this class carries no tunable data, it's a marker
// #endregion

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_Lick.generated.h"

UCLASS(meta = (DisplayName = "Cat Skill: Lick"))
class MYPROJECT_API UAnimNotify_Lick : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
