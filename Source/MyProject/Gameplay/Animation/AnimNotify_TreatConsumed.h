#pragma once
// #region SUMMARY
// File: AnimNotify_TreatConsumed.h
// Purpose: Phase 11 end-of-sequence marker (section 20) — placed on
//          AM_Cat_LickTreat at the frame the treat is fully eaten. Unlike
//          TreatTouch/Lick this one DOES change gameplay state (the item is
//          consumed) — that's Gameplay, not Presentation, per the Golden
//          Rule, so it belongs here in C++ rather than left to Blueprint.
// Depends on: UAttachmentComponent (Equipment/AttachmentComponent.h)
// Consumed by: AM_Cat_LickTreat (Animation Montage, authored in-editor)
// Exposed params: none — finds the owner's AttachmentComponent automatically
// #endregion

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TreatConsumed.generated.h"

UCLASS(meta = (DisplayName = "Cat Skill: Treat Consumed"))
class MYPROJECT_API UAnimNotify_TreatConsumed : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
