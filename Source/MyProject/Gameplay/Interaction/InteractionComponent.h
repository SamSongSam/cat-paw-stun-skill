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
// Exposed params: AcceptanceRadius — EditDefaultsOnly per component instance, so a different
//                 actor (bigger enemy, different player character) can be tuned to its own
//                 capsule/mesh size instead of sharing one fixed "arrived" distance. MoveSpeed
//                 multiplier is left to CharacterMovementComponent — not duplicated here.
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
	// logs a warning and no-ops otherwise instead of crashing. Uses this
	// instance's own AcceptanceRadius (below), not a value the caller passes in —
	// see that property's comment for why.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void MoveToActorAndDeliver(AActor* Target);

protected:
	// Was a hardcoded function-default-parameter literal (100.0f) shared by every actor that ever
	// called MoveToActorAndDeliver, regardless of that actor's own size — a bigger enemy or a
	// different player character would silently keep using the same "arrived" distance instead of
	// one sized for its own capsule/mesh. Now per-instance so each Blueprint sets its own.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0", Units = "cm"))
	float AcceptanceRadius = 100.0f;

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
