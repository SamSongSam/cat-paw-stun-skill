// #region SUMMARY
// File: CatPawProjectile.cpp
// Purpose: Implements homing movement (via ProjectileMovementComponent's
//          HomingTargetComponent) and on-hit effect delivery. Ignores the
//          spawning skill's own owner via CollisionComponent's ignore list
//          (set by CatSkillComponent::SpawnProjectile) so the projectile
//          cannot hit whoever cast the skill.
// Depends on: CatPawProjectile.h, EffectReceiverComponent.h
// Consumed by: n/a (module compile unit)
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "CatPawProjectile.h"
#include "EffectReceiverComponent.h"
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

	if (IsValid(ImpactSystem))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactSystem, Hit.Location);
	}

	OnProjectileImpact(OtherActor);
	Destroy();
}
