#pragma once
// #region SUMMARY
// File: GameplayEffectTypes.h
// Purpose: Shared vocabulary every gameplay system talks in (CatSkillComponent,
//          PawProjectile, EffectReceiverComponent, and every later skill). Per
//          CatSkill_test.md section 7: skills never call StatusComponent or
//          EconomyComponent directly — they build an FGameplayEffectSpec and
//          hand it to whatever EffectReceiverComponent they hit.
//          Type is an FGameplayTag, not a C++ enum — designers pick/author
//          effect types from the Gameplay Tags tree in the Editor (Project
//          Settings > Gameplay Tags, or the tag picker on any FGameplayEffectSpec
//          field), the same way a Unity Inspector enum dropdown works, except
//          new effect types can be added there without a C++ recompile. The
//          fixed set EffectReceiverComponent currently knows how to actually
//          act on is native-registered in CatGameplayTags.h/.cpp — adding a
//          tag in the Editor alone does not give it behavior, it still needs a
//          case in EffectReceiverComponent::ReceiveEffect (that dispatch logic
//          is Gameplay -> C++ per the Golden Rule, and can't be data-only).
// Depends on: GameplayTags module (see INTEGRATION_NOTES.md section 2)
// Consumed by: UEffectReceiverComponent (dispatches by Type), UCatSkillData
//              (Effects[] array), UCatSkillComponent / APawProjectile (build specs)
// Exposed params: n/a — plain struct, fields are BlueprintReadWrite so
//                 Blueprint can build/inspect specs too
// #endregion

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.generated.h"

class UCarryItemData;

// Section 7: "Type / Magnitude / Duration / Target / Source / Asset".
// Not every field is used by every Type (e.g. Magnitude is meaningless for
// Stun) — EffectReceiverComponent reads only the fields its Type cares about.
USTRUCT(BlueprintType)
struct FGameplayEffectSpec
{
	GENERATED_BODY()

	// Editor-editable via the Gameplay Tag picker (tree view, searchable, just like an Inspector
	// enum dropdown) — see CatGameplayTags.h for the tags EffectReceiverComponent currently
	// dispatches on (Effect.Stun, Effect.GoldCost, Effect.CarryItem, Effect.MoveToActor,
	// Effect.TransferItem).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FGameplayTag Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float Magnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float Duration = 0.0f;

	// Filled at runtime by whoever sends the effect — not authored on the Data Asset.
	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Effect")
	TObjectPtr<AActor> Source = nullptr;

	// Used by CarryItem/TransferItem — which item to spawn/attach.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TObjectPtr<UCarryItemData> Asset = nullptr;
};
