// #region SUMMARY
// File: StatusComponent.cpp
// Purpose: Implements ApplyStun/RemoveStun. Stops CharacterMovement while
//          stunned (if the owner is a Character) and restores it after —
//          "disable relevant AI" (section 10) is left to Blueprint/AIController
//          via OnStunStarted/OnStunEnded, since AI behavior is not this
//          component's job (Golden Rule: AI behavior -> AI/Interaction Component).
// Depends on: StatusComponent.h
// Consumed by: n/a (module compile unit)
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "StatusComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UStatusComponent::UStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStatusComponent::ApplyStun(float Duration)
{
	// --- Step 1: escalation counter always advances, independent of whether this call actually
	// starts a new stun or lands on an already-stunned target. This is deliberate: "repeated hits
	// escalate" (NEXT.md section 3) doesn't say the target has to be un-stunned between hits.
	if (StunStack < MaxStunStack)
	{
		++StunStack;
		OnStunStackChanged(StunStack);

		if (StunStack == MaxStunStack)
		{
			// Edge-triggered — fires exactly once per climb to the ceiling, not on every
			// subsequent hit while already at MaxStunStack.
			OnStunStackMaxed();
		}
	}

	// --- Step 2: stun state itself. Existing stun is left running as-is (no-op, no timer restart)
	// so a fast second hit can't refresh/extend the duration indefinitely.
	if (bIsStunned)
	{
		return;
	}

	bIsStunned = true;

	// Movement lock only — "disable relevant AI" is intentionally left to Blueprint/AIController
	// via OnStunStarted below (Golden Rule: AI behavior belongs in AI/Interaction Component, not here).
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
		{
			Movement->DisableMovement();
		}
	}

	GetWorld()->GetTimerManager().SetTimer(StunTimerHandle, this, &UStatusComponent::RemoveStun, Duration, false);

	OnStunStarted(Duration);
}

void UStatusComponent::RemoveStun()
{
	// Timer-driven callback (see ApplyStun's SetTimer call) — guard against the timer somehow
	// firing on an actor that isn't stunned (e.g. ResetStunStack racing with the timer is not
	// possible today since nothing calls it, but this guard costs nothing and prevents a stray
	// OnStunEnded broadcast if that ever changes).
	if (!bIsStunned)
	{
		return;
	}

	bIsStunned = false;

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}

	OnStunEnded();
}

void UStatusComponent::ResetStunStack()
{
	// Unconditional — safe to call even if StunStack is already 0 (e.g. a future payoff phase
	// calling this defensively). Always broadcasts so listeners don't have to special-case "reset
	// from zero" vs "reset from a real value".
	StunStack = 0;
	OnStunStackChanged(StunStack);
}
