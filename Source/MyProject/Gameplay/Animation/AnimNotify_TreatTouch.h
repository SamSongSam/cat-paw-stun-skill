#pragma once
// #region SUMMARY
// File: AnimNotify_TreatTouch.h
// Purpose: Phase 11 timing marker (section 20) — placed on AM_Cat_ReceiveTreat
//          at the frame the treat visually reaches the cat's mouth. Pure
//          timing signal; Blueprint/AnimBP binds "AnimNotify_TreatTouch" to
//          play its own VFX/SFX (Golden Rule: Animation/Presentation -> Blueprint).
// Depends on: none beyond UAnimNotify
// Consumed by: AM_Cat_ReceiveTreat (Animation Montage, authored in-editor)
// Exposed params: none — this class carries no tunable data, it's a marker
// #endregion

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TreatTouch.generated.h"

UCLASS(meta = (DisplayName = "Cat Skill: Treat Touch"))
class MYPROJECT_API UAnimNotify_TreatTouch : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
