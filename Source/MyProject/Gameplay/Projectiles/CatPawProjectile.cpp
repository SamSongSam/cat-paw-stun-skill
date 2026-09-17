// #region SUMMARY
// File: CatPawProjectile.cpp
// Purpose: Implements homing movement (via ProjectileMovementComponent's
//          HomingTargetComponent) and on-hit effect delivery. Ignores the
//          spawning skill's own owner via CollisionComponent's ignore list
//          (set by CatSkillComponent::SpawnProjectile) so the projectile
//          cannot hit whoever cast the skill. Also implements the Dynamic FX
//          parameter push (NEXT.md sections 4-5, 10) — see header.
// Depends on: CatPawProjectile.h, EffectReceiverComponent.h, StatusComponent.h,
//             CatFXParamNames.h
// Consumed by: n/a (module compile unit)
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "CatPawProjectile.h"
#include "EffectReceiverComponent.h"
#include "StatusComponent.h"
#include "CatFXParamNames.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

ACatPawProjectile::ACatPawProjectile()
{
	// Event-driven only (OnComponentHit) — no per-frame gameplay work needed, matches the
	// project's "no Tick polling" rule.
	PrimaryActorTick.bCanEverTick = false;

	// Root: a small blocking sphere is the actual collision/movement anchor — the Niagara visual
	// is attached to it, not the other way around, so the paw's hitbox never depends on FX scale.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	SetRootComponent(CollisionComponent);

	// Placeholder shape for now (Phase 4/5) — Phase 12's procedural SDF paw Material swaps only
	// what this system's Material looks like, not this component wiring.
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(CollisionComponent);

	// Homing dash toward whatever InitializeProjectile sets as HomingTargetComponent.
	// HomingAccelerationMagnitude is the prototype default — designers retune it on
	// BP_PawProjectile (EditDefaultsOnly), never by editing this constructor. Initial/MaxSpeed
	// below are only a preview/fallback value for looking at this actor in isolation (e.g. dropped
	// into a level directly) — InitializeProjectile always overwrites both from
	// SkillData->ProjectileSpeed once a real skill spawns this projectile, so the number that
	// actually matters in-game lives on the Data Asset, not here.
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingAccelerationMagnitude = 8000.0f;
}

void ACatPawProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Bound here (not the constructor) because OnComponentHit needs a live UWorld to fire in.
	CollisionComponent->OnComponentHit.AddDynamic(this, &ACatPawProjectile::OnHit);

	// GetInstigator() is the caster (set via SpawnParams.Instigator in
	// CatSkillComponent::SpawnProjectile) — ignored so the paw can't immediately register a hit
	// against the cat that just threw it.
	if (IsValid(GetInstigator()))
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}

void ACatPawProjectile::InitializeProjectile(AActor* InTargetActor, const TArray<FGameplayEffectSpec>& InEffectsToApply, float LaunchSpeed)
{
	TargetActor = InTargetActor;
	EffectsToApply = InEffectsToApply;

	// Overwrites the constructor's default Initial/MaxSpeed with this specific cast's skill-defined
	// speed (UCatSkillData::ProjectileSpeed). Without this, every projectile from every skill would
	// silently move at whatever number happened to be hardcoded in this actor's C++ constructor,
	// no matter what a designer set on the Data Asset — the exact "changing the config doesn't
	// change the behavior" bug this function signature is written to prevent (see header comment).
	if (IsValid(ProjectileMovement) && LaunchSpeed > 0.0f)
	{
		ProjectileMovement->InitialSpeed = LaunchSpeed;
		ProjectileMovement->MaxSpeed = LaunchSpeed;
	}
	else if (IsValid(ProjectileMovement))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatPawProjectile] %s: InitializeProjectile got LaunchSpeed <= 0 "
			"(%.1f) — keeping the component's existing Initial/MaxSpeed instead of zeroing it out."),
			*GetName(), LaunchSpeed);
	}

	// Homing needs a live scene component to track, not just a location snapshot — this is what
	// makes the paw continue curving toward a moving target instead of a fixed point.
	if (IsValid(TargetActor) && IsValid(ProjectileMovement))
	{
		USceneComponent* TargetRoot = TargetActor->GetRootComponent();
		if (IsValid(TargetRoot))
		{
			ProjectileMovement->HomingTargetComponent = TargetRoot;
		}
	}

	// Dynamic FX contract (NEXT.md section 5) — distance/velocity are fixed once here, at cast
	// time, not re-evaluated every frame (this actor doesn't Tick). Must run AFTER the speed
	// override above so AttackVelocity's fallback (see UpdateDynamicMotionParameters) reflects
	// this skill's actual speed, not the constructor default.
	UpdateDynamicMotionParameters();
}

void ACatPawProjectile::UpdateDynamicMotionParameters()
{
	if (!IsValid(NiagaraComponent) || !IsValid(ProjectileMovement))
	{
		return;
	}

	const FVector TargetLocation = IsValid(TargetActor) ? TargetActor->GetActorLocation() : GetActorLocation();
	const float Distance = FVector::Dist(GetActorLocation(), TargetLocation);

	// ProjectileMovement->Velocity is not guaranteed to be populated yet at this point in the
	// spawn sequence (it derives Velocity from InitialSpeed on its own BeginPlay/first tick) — fall
	// back to direction * InitialSpeed, which is what it will converge to, so AttackVelocity is
	// never a stale zero vector for the trail's first frame.
	const FVector Velocity = ProjectileMovement->Velocity.IsNearlyZero()
		? GetActorForwardVector() * ProjectileMovement->InitialSpeed
		: ProjectileMovement->Velocity;

	NiagaraComponent->SetVariableVec3(CatFXParamNames::TargetPosition, TargetLocation);
	NiagaraComponent->SetVariableFloat(CatFXParamNames::DistanceToTarget, Distance);
	NiagaraComponent->SetVariableVec3(CatFXParamNames::AttackVelocity, Velocity);
}

void ACatPawProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// OtherActor == GetInstigator() should already be excluded by IgnoreActorWhenMoving in
	// BeginPlay, but that only suppresses the physics collision — this is the authoritative
	// gameplay-side guard against ever delivering effects back onto the caster.
	if (!IsValid(OtherActor) || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	UEffectReceiverComponent* Receiver = OtherActor->FindComponentByClass<UEffectReceiverComponent>();
	if (IsValid(Receiver))
	{
		// A copy per-iteration (FGameplayEffectSpec by value) because Source/Target are filled in
		// here per-delivery — EffectsToApply itself stays untouched so a future re-hit (if this
		// projectile ever pierces instead of destroying itself) would still have clean specs.
		for (FGameplayEffectSpec Effect : EffectsToApply)
		{
			Effect.Source = GetInstigator();
			Effect.Target = OtherActor;
			Receiver->ReceiveEffect(Effect);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatPawProjectile] Hit %s but it has no EffectReceiverComponent — "
			"no effects applied."), *GetNameSafe(OtherActor));
	}

	// Read StunStack after ReceiveEffect so the impact reflects the stack this exact hit produced
	// (Stun effects, if present in EffectsToApply, already incremented it above via ApplyStun).
	const UStatusComponent* HitStatus = IsValid(OtherActor) ? OtherActor->FindComponentByClass<UStatusComponent>() : nullptr;
	const int32 HitStunStack = IsValid(HitStatus) ? HitStatus->GetStunStack() : 0;
	// Fallback of 1 (not 0) when there's no StatusComponent — HitStunStack is already 0 in that
	// case, so 0 / 1 still yields Intensity 0.0 without risking a divide-by-zero.
	const int32 HitMaxStunStack = IsValid(HitStatus) ? HitStatus->GetMaxStunStack() : 1;

	SpawnImpactFX(Hit, HitStunStack, HitMaxStunStack);

	OnProjectileImpact(OtherActor);
	Destroy();
}

void ACatPawProjectile::SpawnImpactFX(const FHitResult& Hit, int32 HitStunStack, int32 HitMaxStunStack) const
{
	if (!IsValid(ImpactSystem))
	{
		return;
	}

	UNiagaraComponent* ImpactComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this, ImpactSystem, Hit.Location, Hit.Normal.Rotation(), FVector(1.0f),
		/*bAutoDestroy=*/true, /*bAutoActivate=*/false);

	if (!IsValid(ImpactComponent))
	{
		return;
	}

	const float Intensity = static_cast<float>(HitStunStack) / static_cast<float>(FMath::Max(HitMaxStunStack, 1));

	ImpactComponent->SetVariableVec3(CatFXParamNames::HitLocation, Hit.Location);
	ImpactComponent->SetVariableVec3(CatFXParamNames::HitNormal, Hit.Normal);
	ImpactComponent->SetVariableInt(CatFXParamNames::StunStack, HitStunStack);
	ImpactComponent->SetVariableFloat(CatFXParamNames::FXIntensity, Intensity);

	ImpactComponent->Activate(true);
}
