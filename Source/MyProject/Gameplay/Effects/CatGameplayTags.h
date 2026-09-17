#pragma once
// #region SUMMARY
// File: CatGameplayTags.h
// Purpose: Native Gameplay Tag declarations for the effect types EffectReceiverComponent actually
//          knows how to dispatch. Replaces the old `enum class EGameplayEffectType` — an
//          FGameplayEffectSpec's Type is now a plain FGameplayTag (GameplayEffectTypes.h), so it's
//          authored from the Editor's tag picker (tree/search UI, same experience as a Unity
//          Inspector enum dropdown) instead of a closed C++ enum. These UE_DECLARE_GAMEPLAY_TAG_EXTERN
//          tags self-register with the engine's Gameplay Tags system on module startup — they show
//          up in Project Settings > Gameplay Tags automatically, no .ini editing required, and a
//          designer can add sibling/child tags there (e.g. Effect.Burn) at any time without a C++
//          change. A new tag only gets no-op behavior until a matching case is added to
//          EffectReceiverComponent::ReceiveEffect — the dispatch logic itself stays C++ per the
//          project's Golden Rule (Gameplay -> C++), only the vocabulary of Type values is data-driven.
// Depends on: GameplayTags module (NativeGameplayTags.h)
// Consumed by: EffectReceiverComponent.cpp (compares FGameplayEffectSpec::Type against these)
// Exposed params: n/a — these are tag identities, not UPROPERTYs
// #endregion

#include "NativeGameplayTags.h"

namespace CatGameplayTags
{
	MYPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Stun);
	MYPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_GoldCost);
	MYPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_CarryItem);
	MYPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_MoveToActor);
	MYPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_TransferItem);
}
