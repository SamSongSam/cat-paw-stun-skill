#pragma once
// #region SUMMARY
// File: GameplayEffectTypes.h
// Purpose: Shared vocabulary every gameplay system talks in (CatSkillComponent,
//          PawProjectile, EffectReceiverComponent, and every later skill). Per
//          CatSkill_test.md section 7: skills never call StatusComponent or
//          EconomyComponent directly — they build an FGameplayEffectSpec and
//          hand it to whatever EffectReceiverComponent they hit.
// Depends on: none (leaf types header)
// Consumed by: UEffectReceiverComponent (dispatches by Type), UCatSkillData
//              (Effects[] array), UCatSkillComponent / APawProjectile (build specs)
// Exposed params: n/a — plain enum + struct, fields are BlueprintReadWrite so
//                 Blueprint can build/inspect specs too
// #endregion

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.generated.h"

class UCarryItemData;

// Section 7: keep this list to what the doc specifies now — add a case only
// when a skill actually needs it, not speculatively.
UENUM(BlueprintType)
enum class EGameplayEffectType : uint8
{
	Stun,
	GoldCost,
	CarryItem,
	MoveToActor,
	TransferItem
};

// Section 7: "Type / Magnitude / Duration / Target / Source / Asset / Tag".
// Not every field is used by every Type (e.g. Magnitude is meaningless for
// Stun) — EffectReceiverComponent reads only the fields its Type cares about.
USTRUCT(BlueprintType)
struct FGameplayEffectSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	EGameplayEffectType Type = EGameplayEffectType::Stun;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FGameplayTag Tag;
};
