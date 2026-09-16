// #region SUMMARY
// File: AnimNotify_TreatTouch.cpp
// Purpose: Forwards to the base UAnimNotify::Notify(), which fires the
//          "Received Notify" Blueprint-native event — designers implement
//          that on a Blueprint child of this class (or the AnimBP) to play
//          the touch VFX/SFX. No gameplay state changes here on purpose.
// Depends on: AnimNotify_TreatTouch.h
// Consumed by: n/a (module compile unit)
// Exposed params: none
// #endregion

#include "AnimNotify_TreatTouch.h"

void UAnimNotify_TreatTouch::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
}
