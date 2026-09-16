// #region SUMMARY
// File: AnimNotify_Lick.cpp
// Purpose: Forwards to the base UAnimNotify::Notify() — see AnimNotify_TreatTouch.cpp
//          for the same pattern/reasoning.
// Depends on: AnimNotify_Lick.h
// Consumed by: n/a (module compile unit)
// Exposed params: none
// #endregion

#include "AnimNotify_Lick.h"

void UAnimNotify_Lick::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
}
