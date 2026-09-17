// #region SUMMARY
// File: CatGameplayTags.cpp
// Purpose: Defines the native tags declared in CatGameplayTags.h — the string literal here (e.g.
//          "Effect.Stun") is the tag's identity as it appears in the Editor's Gameplay Tags tree.
// Depends on: CatGameplayTags.h
// Consumed by: n/a (module compile unit)
// Exposed params: none
// #endregion

#include "CatGameplayTags.h"

namespace CatGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Effect_Stun, "Effect.Stun");
	UE_DEFINE_GAMEPLAY_TAG(Effect_GoldCost, "Effect.GoldCost");
	UE_DEFINE_GAMEPLAY_TAG(Effect_CarryItem, "Effect.CarryItem");
	UE_DEFINE_GAMEPLAY_TAG(Effect_MoveToActor, "Effect.MoveToActor");
	UE_DEFINE_GAMEPLAY_TAG(Effect_TransferItem, "Effect.TransferItem");
}
