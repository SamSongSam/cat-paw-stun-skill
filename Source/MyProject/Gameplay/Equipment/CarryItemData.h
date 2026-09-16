#pragma once
// #region SUMMARY
// File: CarryItemData.h
// Purpose: Data-driven description of one carryable item (section 14) — e.g.
//          DA_CatTreat. Skill/AI code never hardcodes "the cat treat mesh" or
//          "attach to hand_r_socket"; it only knows CarryItem = this asset.
// Depends on: EAttachmentSlot (AttachmentComponent.h)
// Consumed by: UAttachmentComponent (EquipItem reads ActorClass/Slot/offsets),
//              FGameplayEffectSpec.Asset (CarryItem/TransferItem effects)
// Exposed params: everything below is EditAnywhere so a designer can author a
//                 new carry item (DA_Sword, DA_Hat, ...) with zero C++ changes
// #endregion

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttachmentComponent.h"
#include "CarryItemData.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class USoundBase;

UCLASS(BlueprintType)
class MYPROJECT_API UCarryItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Info")
	FName ItemID;

	// The actor spawned to represent this item in the world (e.g. BP_CatTreat).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Info")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Attachment")
	EAttachmentSlot AttachmentSlot = EAttachmentSlot::RightHand;

	// Overrides the slot's default socket when set to something other than
	// NAME_None — most items should just rely on the slot->socket map instead.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Attachment")
	FName SocketNameOverride = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Attachment")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Attachment")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Attachment")
	FVector Scale = FVector::OneVector;

	// Presentation-only references — Blueprint/Animation reads these, C++
	// (AttachmentComponent) never plays them itself (see Golden Rule).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Presentation")
	TObjectPtr<UAnimMontage> PickupAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Presentation")
	TObjectPtr<UAnimMontage> CarryAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Presentation")
	TObjectPtr<UAnimMontage> GiveAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Presentation")
	TObjectPtr<UNiagaraSystem> NiagaraFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Presentation")
	TObjectPtr<USoundBase> Sound;
};
