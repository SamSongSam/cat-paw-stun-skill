// #region SUMMARY
// File: EffectReceiverComponent.cpp
// Purpose: Implements ReceiveEffect's dispatch switch (section 9). TransferItem
//          is intentionally NOT handled here — UInteractionComponent already
//          performs the attachment transfer itself once it reaches the target
//          (see InteractionComponent.cpp::HandleMoveCompleted), so routing it
//          here too would just be duplicate logic for nothing this project needs.
// Depends on: EffectReceiverComponent.h, StatusComponent.h, EconomyComponent.h,
//             AttachmentComponent.h, InteractionComponent.h
// Consumed by: n/a (module compile unit)
// Exposed params: none
// #endregion

#include "EffectReceiverComponent.h"
#include "StatusComponent.h"
#include "EconomyComponent.h"
#include "AttachmentComponent.h"
#include "InteractionComponent.h"
#include "CarryItemData.h"

UEffectReceiverComponent::UEffectReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEffectReceiverComponent::ReceiveEffect(const FGameplayEffectSpec& Effect)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	OnEffectReceived(Effect);

	switch (Effect.Type)
	{
	case EGameplayEffectType::Stun:
	{
		UStatusComponent* Status = Owner->FindComponentByClass<UStatusComponent>();
		if (IsValid(Status))
		{
			Status->ApplyStun(Effect.Duration);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EffectReceiver] %s has no StatusComponent — Stun effect dropped."),
				*GetNameSafe(Owner));
		}
		break;
	}

	case EGameplayEffectType::GoldCost:
	{
		UEconomyComponent* Economy = Owner->FindComponentByClass<UEconomyComponent>();
		if (IsValid(Economy))
		{
			Economy->SpendGold(FMath::RoundToInt(Effect.Magnitude));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EffectReceiver] %s has no EconomyComponent — GoldCost effect dropped."),
				*GetNameSafe(Owner));
		}
		break;
	}

	case EGameplayEffectType::CarryItem:
	{
		UAttachmentComponent* Attachment = Owner->FindComponentByClass<UAttachmentComponent>();
		if (IsValid(Attachment) && IsValid(Effect.Asset))
		{
			Attachment->EquipItem(Effect.Asset);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EffectReceiver] %s: CarryItem effect dropped (missing "
				"AttachmentComponent or Effect.Asset)."), *GetNameSafe(Owner));
		}
		break;
	}

	case EGameplayEffectType::MoveToActor:
	{
		// Effect.Target is the actor RECEIVING this effect (Owner, the enemy
		// that got hit) — the destination to walk to is Effect.Source (the
		// skill caster). Using Effect.Target here would make the enemy walk
		// to itself and silently do nothing.
		UInteractionComponent* Interaction = Owner->FindComponentByClass<UInteractionComponent>();
		if (IsValid(Interaction) && IsValid(Effect.Source))
		{
			Interaction->MoveToActorAndDeliver(Effect.Source);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EffectReceiver] %s: MoveToActor effect dropped (missing "
				"InteractionComponent or Effect.Source)."), *GetNameSafe(Owner));
		}
		break;
	}

	case EGameplayEffectType::TransferItem:
		// See file header — handled by InteractionComponent on reaching the target.
		break;
	}
}
