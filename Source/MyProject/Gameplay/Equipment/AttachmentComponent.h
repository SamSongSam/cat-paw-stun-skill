#pragma once
// #region SUMMARY
// File: AttachmentComponent.h
// Purpose: Generic "this actor is holding an item" component (sections 12-13,
//          18-19). Owns the socket map so nobody hardcodes bone names outside
//          this one place. Used by BOTH enemies (carry the treat) and the
//          player (receive the treat) — same component, same API either way.
// Depends on: UCarryItemData (Equipment/CarryItemData.h)
// Consumed by: UEffectReceiverComponent (CarryItem effect calls EquipItem),
//              UInteractionComponent (TransferItem calls TransferTo)
// Exposed params: SlotSockets — designer can remap slot->socket per skeleton
//                 without touching C++
// #endregion

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttachmentComponent.generated.h"

class UCarryItemData;

// Section 13 — semantic slots, not raw bone names.
UENUM(BlueprintType)
enum class EAttachmentSlot : uint8
{
	RightHand,
	LeftHand,
	Head,
	Mouth,
	Back
};

UCLASS(ClassGroup = (CatSkill), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UAttachmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttachmentComponent();

	// Spawns ItemData->ActorClass and attaches it to this actor's mesh at the
	// slot's socket. No-op (with a warning) if this actor already holds an
	// item or has no skeletal mesh to attach to.
	UFUNCTION(BlueprintCallable, Category = "Attachment")
	AActor* EquipItem(UCarryItemData* ItemData);

	// Detaches the held item from this actor without destroying it — used
	// right before TransferTo hands it to another AttachmentComponent.
	UFUNCTION(BlueprintCallable, Category = "Attachment")
	AActor* DetachItem();

	// Moves the currently held item from this component to Other in one
	// step (detach here, attach there) — see section 18: "Item ตัวเดิมย้าย
	// holder ไม่จำเป็นต้อง destroy แล้ว spawn ใหม่".
	UFUNCTION(BlueprintCallable, Category = "Attachment")
	bool TransferTo(UAttachmentComponent* Other);

	UFUNCTION(BlueprintPure, Category = "Attachment")
	bool IsHoldingItem() const { return IsValid(HeldItemActor); }

	UFUNCTION(BlueprintPure, Category = "Attachment")
	AActor* GetHeldItemActor() const { return HeldItemActor; }

	UFUNCTION(BlueprintPure, Category = "Attachment")
	UCarryItemData* GetHeldItemData() const { return HeldItemData; }

protected:
	// Designer-tunable per skeleton — see Golden Rule: this is data, not code.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	TMap<EAttachmentSlot, FName> SlotSockets = {
		{EAttachmentSlot::RightHand, TEXT("hand_r_socket")},
		{EAttachmentSlot::LeftHand, TEXT("hand_l_socket")},
		{EAttachmentSlot::Head, TEXT("head_socket")},
		{EAttachmentSlot::Mouth, TEXT("mouth_socket")},
		{EAttachmentSlot::Back, TEXT("back_socket")},
	};

	UPROPERTY(BlueprintReadOnly, Category = "Attachment")
	TObjectPtr<AActor> HeldItemActor;

	UPROPERTY(BlueprintReadOnly, Category = "Attachment")
	TObjectPtr<UCarryItemData> HeldItemData;

	// Presentation hooks — Blueprint plays PickupAnimation/GiveAnimation etc.
	// from ItemData, C++ never plays animations itself (Golden Rule).
	UFUNCTION(BlueprintImplementableEvent, Category = "Attachment")
	void OnItemEquipped(UCarryItemData* ItemData, AActor* ItemActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Attachment")
	void OnItemDetached(UCarryItemData* ItemData, AActor* ItemActor);
};
