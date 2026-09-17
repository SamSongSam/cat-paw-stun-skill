#pragma once
// #region SUMMARY
// File: CatPawProjectile.h
// Purpose: Prototype paw projectile (section 28) — placeholder shape for now
//          (Phase 4/5); the procedural SDF paw material (Phase 12) replaces
//          only the Niagara/Material look, this actor's gameplay does not
//          change. Homes toward TargetActor, sends EffectSpecs to whatever
//          it hits, then destroys itself. Also pushes the Dynamic FX
//          parameter contract (NEXT.md sections 4-5, 10) onto its own trail
//          NiagaraComponent at cast time and onto ImpactSystem at hit time.
// Depends on: UEffectReceiverComponent (target must have one to receive
//             EffectSpecs), FGameplayEffectSpec (GameplayEffectTypes.h),
//             UStatusComponent (reads target's StunStack for impact FX),
//             CatFXParamNames.h (shared Niagara User Parameter names)
// Consumed by: UCatSkillComponent::SpawnProjectile()
// Exposed params: HomingAccelerationMagnitude — EditDefaultsOnly on BP_PawProjectile (this
//                 prototype's own per-flight-feel tunable). ProjectileMovement's Initial/MaxSpeed
//                 are ALSO editable per-BP as a fallback/preview value, but InitializeProjectile
//                 always overwrites them from SkillData->ProjectileSpeed at spawn time — so the
//                 actual in-game speed is Data Asset-driven per skill, not per-Blueprint.
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

	// #region Public API
	// Called by CatSkillComponent right after SpawnActor — sets homing target,
	// the effects to deliver on hit, and this cast's launch speed. Not
	// constructor params because SpawnActor's deferred-construction API adds
	// complexity this prototype does not need yet. LaunchSpeed has no default
	// value on purpose (see this project's code-style rule against baking a
	// shared constant into a function signature — CLAUDE.md rule 3) — the
	// caller must always pass SkillData->ProjectileSpeed explicitly so this
	// projectile can never silently fall back to some other skill's number.
	UFUNCTION(BlueprintCallable, Category = "Cat Paw Projectile")
	void InitializeProjectile(AActor* InTargetActor, const TArray<FGameplayEffectSpec>& InEffectsToApply, float LaunchSpeed);
	// #endregion

protected:
	virtual void BeginPlay() override;

	// #region Collision / hit handling
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& Hit);
	// #endregion

	// #region Dynamic FX helpers (NEXT.md sections 4-5, 10 — see CatFXParamNames.h for the contract)
	// Pushes TargetPosition/DistanceToTarget/AttackVelocity onto NiagaraComponent. Called once from
	// InitializeProjectile since distance/velocity are fixed at cast time (NEXT.md section 5) —
	// this is not a Tick function, the paw does not need to re-evaluate distance mid-flight.
	void UpdateDynamicMotionParameters();

	// Spawns ImpactSystem with HitLocation/HitNormal/StunStack/FXIntensity pre-set as User
	// Parameters. Niagara only reads initial User Parameter values at spawn, so this cannot use
	// the plain SpawnSystemAtLocation-then-set-later pattern — spawn deferred (bAutoActivate=false),
	// set params, then Activate(). HitMaxStunStack is passed in separately (rather than re-querying
	// a component here) because it comes from the hit actor's own StatusComponent instance — this
	// function only knows the two ints OnHit already resolved.
	void SpawnImpactFX(const FHitResult& Hit, int32 HitStunStack, int32 HitMaxStunStack) const;
	// #endregion

	// #region Components (all subobjects created in the constructor)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat Paw Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	// The paw's own trail/main-shape Niagara system (Phase 12 will replace its Material with a
	// procedural SDF paw — this component itself does not change when that happens).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat Paw Projectile")
	TObjectPtr<UNiagaraComponent> NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat Paw Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	// #endregion

	// #region Tunables and runtime state
	// Presentation asset — assigned on the Blueprint/spawner, not hardcoded.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cat Paw Projectile")
	TObjectPtr<UNiagaraSystem> ImpactSystem;

	// Set once in InitializeProjectile; may be nullptr if the caster found no target (see
	// CatSkillComponent::ActivateCatSkill — the projectile is only ever spawned when a target
	// WAS found, but this stays nullable defensively rather than assuming that invariant forever).
	UPROPERTY(BlueprintReadOnly, Category = "Cat Paw Projectile")
	TObjectPtr<AActor> TargetActor;

	// Copied in from CatSkillComponent::BuildEffectSpecs — delivered to whatever this hits, once,
	// in OnHit. Never mutated after InitializeProjectile sets it.
	UPROPERTY(BlueprintReadOnly, Category = "Cat Paw Projectile")
	TArray<FGameplayEffectSpec> EffectsToApply;
	// #endregion

	// #region Presentation hooks (Golden Rule: FX/animation reacts, this class does
	// not play sound/VFX beyond spawning ImpactSystem itself)
	UFUNCTION(BlueprintImplementableEvent, Category = "Cat Paw Projectile")
	void OnProjectileImpact(AActor* HitActor);
	// #endregion
};
