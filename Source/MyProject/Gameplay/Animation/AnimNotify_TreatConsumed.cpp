// #region SUMMARY
// File: AnimNotify_TreatConsumed.cpp
// Purpose: Finds the mesh owner's AttachmentComponent, detaches the held
//          item, and destroys it — "Consume Item -> Detach -> Destroy" from
//          section 20's Cat Lick sequence. No-op (with a warning) if the
//          owner has no AttachmentComponent or isn't holding anything —
//          never assume the designer wired the montage to the right actor.
// Depends on: AnimNotify_TreatConsumed.h, AttachmentComponent.h
// Consumed by: n/a (module compile unit)
// Exposed params: none
// #endregion

#include "AnimNotify_TreatConsumed.h"
#include "AttachmentComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_TreatConsumed::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp))
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	UAttachmentComponent* Attachment = Owner->FindComponentByClass<UAttachmentComponent>();
	if (!IsValid(Attachment) || !Attachment->IsHoldingItem())
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] AnimNotify_TreatConsumed on %s: no AttachmentComponent "
			"or nothing held — nothing to consume."), *Owner->GetName());
		return;
	}

	AActor* ConsumedItem = Attachment->DetachItem();
	if (IsValid(ConsumedItem))
	{
		ConsumedItem->Destroy();
	}
}
