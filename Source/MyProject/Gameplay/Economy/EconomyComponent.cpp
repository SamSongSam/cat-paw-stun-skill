// #region SUMMARY
// File: EconomyComponent.cpp
// Purpose: Implements SpendGold/AddGold, clamped to [0, MaxGold].
// Depends on: EconomyComponent.h
// Consumed by: n/a (module compile unit)
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "EconomyComponent.h"

UEconomyComponent::UEconomyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UEconomyComponent::SpendGold(int32 Amount)
{
	if (!CanAfford(Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Economy] %s cannot afford %d gold (has %d)."),
			*GetNameSafe(GetOwner()), Amount, CurrentGold);
		return false;
	}

	CurrentGold -= Amount;
	OnGoldSpent(Amount, CurrentGold);
	return true;
}

void UEconomyComponent::AddGold(int32 Amount)
{
	CurrentGold = FMath::Clamp(CurrentGold + Amount, 0, MaxGold);
}
