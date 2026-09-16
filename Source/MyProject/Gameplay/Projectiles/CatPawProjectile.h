#pragma once
// #region SUMMARY
// File: CatPawProjectile.h
// Purpose: Prototype paw projectile (section 28) — placeholder shape for now
//          (Phase 4/5); the procedural SDF paw material (Phase 12) replaces
//          only the Niagara/Material look, this actor's gameplay does not
//          change. Homes toward TargetActor, sends EffectSpecs to whatever
//          it hits, then destroys itself.
// Depends on: UEffectReceiverComponent (target must have one to receive
//             EffectSpecs), FGameplayEffectSpec (GameplayEffectTypes.h)
// Consumed by: UCatSkillComponent::SpawnProjectile()
// Exposed params: Speed, HomingAcceleration — EditDefaultsOnly on
//                 BP_PawProjectile so VFX/gameplay can tune dash feel
//                 without a C++ recompile
// #endregion

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "CatPawProjectile.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;

UCLASS()
class MYPROJECT_API ACatPawProjectile : public AActor
{
	GENERATED_BODY()

public:
	ACatPawProjectile();

	// Called by CatSkillComponent right after SpawnActor — sets homing target
	// and the effects to deliver on hit. Not a constructor param because
	// SpawnActor's deferred-construction API adds complexity this prototype
	// does not need yet.
	UFUNCTION(BlueprintCallable, Category = "Cat Paw Projectile")
	void InitializeProjectile(AActor* InTargetActor, const TArray<FGameplayEffectSpec>& InEffectsToApply);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat Paw Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat Paw Projectile")
	TObjectPtr<UNiagaraComponent> NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat Paw Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// Presentation asset — assigned on the Blueprint/spawner, not hardcoded.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cat Paw Projectile")
	TObjectPtr<UNiagaraSystem> ImpactSystem;

	UPROPERTY(BlueprintReadOnly, Category = "Cat Paw Projectile")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(BlueprintReadOnly, Category = "Cat Paw Projectile")
	TArray<FGameplayEffectSpec> EffectsToApply;

	// Presentation hooks (Golden Rule: FX/animation reacts, this class does
	// not play sound/VFX beyond spawning ImpactSystem itself).
	UFUNCTION(BlueprintImplementableEvent, Category = "Cat Paw Projectile")
	void OnProjectileImpact(AActor* HitActor);
};
