// #region SUMMARY
// File: InteractionComponent.cpp
// Purpose: Implements MoveToActorAndDeliver via AAIController::MoveToActor +
//          PathFollowingComponent's OnRequestFinished delegate — event-driven,
//          no Tick polling. On success, transfers the held item to Target's
//          AttachmentComponent (section 18) and fires the presentation events.
// Depends on: InteractionComponent.h, AttachmentComponent.h
// Consumed by: n/a (module compile unit)
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "InteractionComponent.h"
#include "AttachmentComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Pawn.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::MoveToActorAndDeliver(AActor* Target)
{
	if (!IsValid(Target))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interaction] %s: MoveToActorAndDeliver called with no Target."),
			*GetNameSafe(GetOwner()));
		return;
	}

	if (bIsMoving)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interaction] %s is already mid-interaction — ignoring new request."),
			*GetNameSafe(GetOwner()));
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AAIController* AIController = IsValid(OwnerPawn) ? Cast<AAIController>(OwnerPawn->GetController()) : nullptr;
	if (!IsValid(AIController))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interaction] %s has no AIController — cannot MoveToActor. "
			"Prototype scope only supports AI-controlled pawns."), *GetNameSafe(GetOwner()));
		return;
	}

	UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent();
	if (IsValid(PathFollowing))
	{
		PathFollowing->OnRequestFinished.AddUObject(this, &UInteractionComponent::HandleMoveCompleted);
	}

	CurrentTarget = Target;
	bIsMoving = true;

	AIController->MoveToActor(Target, AcceptanceRadius);
	OnInteractionStarted(Target);
}

void UInteractionComponent::HandleMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (!bIsMoving || !IsValid(CurrentTarget))
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AAIController* AIController = IsValid(OwnerPawn) ? Cast<AAIController>(OwnerPawn->GetController()) : nullptr;
	if (IsValid(AIController) && IsValid(AIController->GetPathFollowingComponent()))
	{
		AIController->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
	}

	AActor* Target = CurrentTarget;
	OnReachedTarget(Target);

	UAttachmentComponent* MyAttachment = GetOwner()->FindComponentByClass<UAttachmentComponent>();
	UAttachmentComponent* TargetAttachment = Target->FindComponentByClass<UAttachmentComponent>();

	if (IsValid(MyAttachment) && IsValid(TargetAttachment) && MyAttachment->IsHoldingItem())
	{
		if (MyAttachment->TransferTo(TargetAttachment))
		{
			OnGiveItem(Target);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interaction] %s reached %s but could not transfer an item "
			"(missing AttachmentComponent on one side, or nothing held)."),
			*GetNameSafe(GetOwner()), *GetNameSafe(Target));
	}

	FinishInteraction();
}

void UInteractionComponent::FinishInteraction()
{
	CurrentTarget = nullptr;
	bIsMoving = false;
	OnInteractionFinished();
}
