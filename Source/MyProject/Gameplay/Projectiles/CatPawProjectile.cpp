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
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	SetRootComponent(CollisionComponent);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(CollisionComponent);

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

	CollisionComponent->OnComponentHit.AddDynamic(this, &ACatPawProjectile::OnHit);

	if (IsValid(GetInstigator()))
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}

void ACatPawProjectile::InitializeProjectile(AActor* InTargetActor, const TArray<FGameplayEffectSpec>& InEffectsToApply)
{
	TargetActor = InTargetActor;
	EffectsToApply = InEffectsToApply;

	if (IsValid(TargetActor) && IsValid(ProjectileMovement))
	{
		USceneComponent* TargetRoot = TargetActor->GetRootComponent();
		if (IsValid(TargetRoot))
		{
			ProjectileMovement->HomingTargetComponent = TargetRoot;
		}
	}

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
	if (!IsValid(OtherActor) || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	UEffectReceiverComponent* Receiver = OtherActor->FindComponentByClass<UEffectReceiverComponent>();
	if (IsValid(Receiver))
	{
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

	SpawnImpactFX(Hit, HitStunStack);

	OnProjectileImpact(OtherActor);
	Destroy();
}

void ACatPawProjectile::SpawnImpactFX(const FHitResult& Hit, int32 HitStunStack) const
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

	const float Intensity = static_cast<float>(HitStunStack) / static_cast<float>(UStatusComponent::MaxStunStack);

	ImpactComponent->SetVariableVec3(CatFXParamNames::HitLocation, Hit.Location);
	ImpactComponent->SetVariableVec3(CatFXParamNames::HitNormal, Hit.Normal);
	ImpactComponent->SetVariableInt(CatFXParamNames::StunStack, HitStunStack);
	ImpactComponent->SetVariableFloat(CatFXParamNames::FXIntensity, Intensity);

	ImpactComponent->Activate(true);
}
