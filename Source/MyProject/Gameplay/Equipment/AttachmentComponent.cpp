// #region SUMMARY
// File: AttachmentComponent.cpp
// Purpose: Implements EquipItem/DetachItem/TransferTo. Attaches to whatever
//          USkeletalMeshComponent the owner exposes — works for enemy and
//          player alike since neither knows about the other's class.
// Depends on: AttachmentComponent.h, CarryItemData.h
// Consumed by: n/a (module compile unit)
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "AttachmentComponent.h"
#include "CarryItemData.h"
#include "Components/SkeletalMeshComponent.h"

UAttachmentComponent::UAttachmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* UAttachmentComponent::EquipItem(UCarryItemData* ItemData)
{
	if (!IsValid(ItemData) || !IsValid(ItemData->ActorClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attachment] %s: EquipItem called with no valid ItemData/ActorClass."),
			*GetNameSafe(GetOwner()));
		return nullptr;
	}

	if (IsHoldingItem())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attachment] %s already holds an item — DetachItem() it first."),
			*GetNameSafe(GetOwner()));
		return nullptr;
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return nullptr;
	}

	USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!IsValid(Mesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attachment] %s has no SkeletalMeshComponent to attach to."),
			*GetNameSafe(Owner));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	AActor* SpawnedItem = GetWorld()->SpawnActor<AActor>(ItemData->ActorClass, FTransform::Identity, SpawnParams);
	if (!IsValid(SpawnedItem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attachment] Failed to spawn %s for %s."),
			*ItemData->ActorClass->GetName(), *GetNameSafe(Owner));
		return nullptr;
	}

	const FName* SocketPtr = SlotSockets.Find(ItemData->AttachmentSlot);
	const FName SocketName = (ItemData->SocketNameOverride != NAME_None)
		? ItemData->SocketNameOverride
		: (SocketPtr ? *SocketPtr : NAME_None);

	SpawnedItem->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	SpawnedItem->SetActorRelativeLocation(ItemData->LocationOffset);
	SpawnedItem->SetActorRelativeRotation(ItemData->RotationOffset);
	SpawnedItem->SetActorRelativeScale3D(ItemData->Scale);

	HeldItemActor = SpawnedItem;
	HeldItemData = ItemData;

	OnItemEquipped(ItemData, SpawnedItem);
	return SpawnedItem;
}

AActor* UAttachmentComponent::DetachItem()
{
	if (!IsHoldingItem())
	{
		return nullptr;
	}

	AActor* DetachedActor = HeldItemActor;
	UCarryItemData* DetachedData = HeldItemData;

	DetachedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	HeldItemActor = nullptr;
	HeldItemData = nullptr;

	OnItemDetached(DetachedData, DetachedActor);
	return DetachedActor;
}

bool UAttachmentComponent::TransferTo(UAttachmentComponent* Other)
{
	if (!IsValid(Other) || !IsHoldingItem())
	{
		return false;
	}

	AActor* Owner = Other->GetOwner();
	if (!IsValid(Owner))
	{
		return false;
	}

	USkeletalMeshComponent* OtherMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!IsValid(OtherMesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attachment] TransferTo target %s has no SkeletalMeshComponent."),
			*GetNameSafe(Owner));
		return false;
	}

	UCarryItemData* ItemData = HeldItemData;
	AActor* ItemActor = DetachItem();
	if (!IsValid(ItemActor))
	{
		return false;
	}

	const FName* SocketPtr = Other->SlotSockets.Find(ItemData->AttachmentSlot);
	const FName SocketName = (ItemData->SocketNameOverride != NAME_None)
		? ItemData->SocketNameOverride
		: (SocketPtr ? *SocketPtr : NAME_None);

	ItemActor->AttachToComponent(OtherMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	ItemActor->SetActorRelativeLocation(ItemData->LocationOffset);
	ItemActor->SetActorRelativeRotation(ItemData->RotationOffset);
	ItemActor->SetActorRelativeScale3D(ItemData->Scale);

	Other->HeldItemActor = ItemActor;
	Other->HeldItemData = ItemData;
	Other->OnItemEquipped(ItemData, ItemActor);

	return true;
}
