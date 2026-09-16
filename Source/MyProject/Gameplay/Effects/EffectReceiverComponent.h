#pragma once
// #region SUMMARY
// File: EffectReceiverComponent.h
// Purpose: The gateway every incoming FGameplayEffectSpec passes through
//          (section 9). This is what CatSkillComponent/PawProjectile target-
//          detects for — "Has EffectReceiverComponent" (section 29) — never
//          a specific enemy class. It looks up sibling components on its own
//          owner and forwards by Type; it never contains gameplay rules itself.
// Depends on: FGameplayEffectSpec (GameplayEffectTypes.h)
// Consumed by: APawProjectile (hit -> ReceiveEffect), UCatSkillComponent
//              (ApplySkillEffects iterates SkillData->Effects and sends them)
// Exposed params: none — this component is pure plumbing, no tunables
// #endregion

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "EffectReceiverComponent.generated.h"

UCLASS(ClassGroup = (CatSkill), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UEffectReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEffectReceiverComponent();

	// Looks up StatusComponent/EconomyComponent/AttachmentComponent/
	// InteractionComponent on GetOwner() as needed and forwards the spec.
	// Logs a warning (does not crash) if the owner is missing whichever
	// sibling component the effect's Type requires — see section 9's
	// pseudocode switch, this mirrors it 1:1.
	UFUNCTION(BlueprintCallable, Category = "Effect")
	void ReceiveEffect(const FGameplayEffectSpec& Effect);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Effect")
	void OnEffectReceived(const FGameplayEffectSpec& Effect);
};
