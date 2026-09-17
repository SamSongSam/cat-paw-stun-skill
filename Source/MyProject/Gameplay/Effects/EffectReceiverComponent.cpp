// #region SUMMARY
// File: EffectReceiverComponent.cpp
// Purpose: Implements ReceiveEffect's dispatch (section 9), now matched by FGameplayTag instead of
//          a closed C++ enum (see GameplayEffectTypes.h / CatGameplayTags.h) — an unrecognized tag
//          (including one a designer added in the Editor with no matching case here yet) logs a
//          warning and no-ops rather than failing to compile. TransferItem is intentionally NOT
//          handled here — UInteractionComponent already performs the attachment transfer itself
//          once it reaches the target (see InteractionComponent.cpp::HandleMoveCompleted), so
//          routing it here too would just be duplicate logic for nothing this project needs.
// Depends on: EffectReceiverComponent.h, StatusComponent.h, EconomyComponent.h,
//             AttachmentComponent.h, InteractionComponent.h, CatGameplayTags.h
// Consumed by: n/a (module compile unit)
// Exposed params: none
// #endregion

#include "EffectReceiverComponent.h"
#include "StatusComponent.h"
#include "EconomyComponent.h"
#include "AttachmentComponent.h"
#include "InteractionComponent.h"
#include "CarryItemData.h"
#include "CatGameplayTags.h"

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

	const FGameplayTag& Type = Effect.Type;

	if (Type == CatGameplayTags::Effect_Stun)
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
	}
	else if (Type == CatGameplayTags::Effect_GoldCost)
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
	}
	else if (Type == CatGameplayTags::Effect_CarryItem)
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
	}
	else if (Type == CatGameplayTags::Effect_MoveToActor)
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
	}
	else if (Type == CatGameplayTags::Effect_TransferItem)
	{
		// See file header — handled by InteractionComponent on reaching the target.
	}
	else
	{
		// Not a compile error — a designer-authored tag this dispatch doesn't know yet (or a typo
		// in the picker). Logged instead of silently dropped so it's diagnosable from the Editor.
		UE_LOG(LogTemp, Warning, TEXT("[EffectReceiver] %s: unrecognized effect Type tag '%s' — no case handles it."),
			*GetNameSafe(Owner), *Type.ToString());
	}
}
