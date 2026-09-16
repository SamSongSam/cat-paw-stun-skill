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
	if (StunStack < MaxStunStack)
	{
		++StunStack;
		OnStunStackChanged(StunStack);

		if (StunStack == MaxStunStack)
		{
			OnStunStackMaxed();
		}
	}

	if (bIsStunned)
	{
		return;
	}

	bIsStunned = true;

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
	StunStack = 0;
	OnStunStackChanged(StunStack);
}
