#pragma once
// #region SUMMARY
// File: InteractionComponent.h
// Purpose: Move-to-actor / give-item flow (sections 16-17). Prototype scope
//          per CatSkill_test.md: uses AIController::MoveToActor (simple nav
//          move, no BehaviorTree) — that is Phase 9's "MoveTo Player" bar
//          fully met without needing a BT asset yet. Swap in a real BT later
//          without touching AttachmentComponent or the skill/effect layer.
// Depends on: UAttachmentComponent (calls TransferTo on reach)
// Consumed by: UEffectReceiverComponent (MoveToActor/TransferItem effects)
// Exposed params: AcceptanceRadius, MoveSpeed multiplier left to
//                 CharacterMovementComponent — not duplicated here
// #endregion

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UAttachmentComponent;
struct FAIRequestID;
struct FPathFollowingResult;

UCLASS(ClassGroup = (CatSkill), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	// Starts moving the owner to Target via its AIController. Requires the
	// owner to be possessed by an AAIController with a NavMesh available —
	// logs a warning and no-ops otherwise instead of crashing.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void MoveToActorAndDeliver(AActor* Target, float AcceptanceRadius = 100.0f);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<AActor> CurrentTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bIsMoving = false;

	// Bound to the AIController's PathFollowingComponent instead of polling
	// distance in Tick (see CatSkill_test.md section 4: no Tick-based
	// checking — same principle applies here, not just to input).
	void HandleMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	void FinishInteraction();

	// Presentation hooks (Golden Rule: this component decides WHEN, Blueprint
	// decides HOW it looks — face-target turn, give-item anim, etc.).
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionStarted(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnReachedTarget(AActor* Target);

	// Called after TransferTo succeeds — Blueprint plays the give/receive
	// animation from CarryItemData here (see AttachmentComponent's own
	// OnItemEquipped/OnItemDetached for the item-level hooks).
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnGiveItem(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteractionFinished();
};
