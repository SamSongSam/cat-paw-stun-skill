// #region SUMMARY
// File: CatSkillComponent.cpp
// Purpose: Implements Phase 1-2 activation flow — cooldown/state check, spawn aura attached to
//          owner, notify Blueprint. See CatSkillComponent.h for scope boundary.
// Depends on: CatSkillData.h, NiagaraFunctionLibrary (engine)
// Consumed by: ACatPlayerCharacter
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "CatSkillComponent.h"
#include "CatSkillData.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "EffectReceiverComponent.h"
#include "StatusComponent.h"
#include "CatFXParamNames.h"
#include "CatGameplayTags.h"
#include "CatPawProjectile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

// Temporary debug helper — see CatPlayerCharacter.cpp for the matching one.
// Remove once Phase 1-2 input is confirmed working end to end.
static void CatSkillDebugMessage(const FString& Message, FColor Color = FColor::Yellow)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.0f, Color, TEXT("[CatSkillDebug] ") + Message);
	}
}

UCatSkillComponent::UCatSkillComponent()
{
	// This component only needs to react to explicit calls (ActivateCatSkill) — no per-frame work.
	PrimaryComponentTick.bCanEverTick = false;
}

void UCatSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!SkillData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] %s has no SkillData assigned — skill will no-op."),
			*GetOwner()->GetName());
	}
}

bool UCatSkillComponent::CanActivateSkill() const
{
	if (!IsValid(SkillData))
	{
		return false;
	}

	if (bIsSkillActive)
	{
		return false;
	}

	if (LastCastTime < 0.0f)
	{
		// Never cast yet this session — cooldown cannot block the first cast.
		return true;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	return (World->GetTimeSeconds() - LastCastTime) >= SkillData->Cooldown;
}

void UCatSkillComponent::ActivateCatSkill()
{
	CatSkillDebugMessage(TEXT("ActivateCatSkill called."), FColor::Cyan);

	if (!CanActivateSkill())
	{
		FString Reason = TEXT("unknown");
		if (!IsValid(SkillData))
		{
			Reason = TEXT("SkillData is invalid");
		}
		else if (bIsSkillActive)
		{
			Reason = TEXT("skill already active");
		}
		else if (const UWorld* World = GetWorld())
		{
			Reason = FString::Printf(TEXT("cooldown not elapsed (%.2fs remaining)"),
				SkillData->Cooldown - (World->GetTimeSeconds() - LastCastTime));
		}
		CatSkillDebugMessage(FString::Printf(TEXT("CanActivateSkill() = false (%s)."), *Reason), FColor::Red);
		return;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	bIsSkillActive = true;
	LastCastTime = World->GetTimeSeconds();

	// Found before SpawnAura (not after, like earlier phases) so the aura's Niagara params can
	// reflect the current target's StunStack on spawn (NEXT.md section 7) — finding the target
	// doesn't depend on the aura existing, so this ordering is free.
	AActor* Target = FindTarget();

	SpawnAura(Target);

	OnCatSkillActivated();

	// From here the caster's own job is done — the projectile (once spawned)
	// carries the effects and resolves hit/stun/gold/deliver/lick on its own
	// timeline (sections 9-20). The skill component does not track that
	// downstream chain; it only owns cast/cooldown state (Golden Rule: this
	// component is Gameplay/state, not AI/animation).

	if (bDebugSkill && IsValid(SkillData))
	{
		DrawDebugSphere(World, GetOwner()->GetActorLocation(), SkillData->Range, 24,
			FColor::Yellow, false, 2.0f, 0, 2.0f);
		if (IsValid(Target))
		{
			DrawDebugSphere(World, Target->GetActorLocation(), 40.0f, 12,
				FColor::Red, false, 2.0f, 0, 3.0f);
			DrawDebugString(World, Target->GetActorLocation() + FVector(0, 0, 100),
				FString::Printf(TEXT("TARGET: %s"), *Target->GetName()), nullptr, FColor::Red, 2.0f);
		}
	}

	if (IsValid(Target))
	{
		OnTargetAcquired(Target);
		SpawnProjectile(Target);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[CatSkill] %s: no target found within range — aura only, no projectile."),
			*GetOwner()->GetName());
	}

	bIsSkillActive = false;
	OnSkillFinished();
}

AActor* UCatSkillComponent::FindTarget() const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(SkillData))
	{
		return nullptr;
	}

	TArray<AActor*> ActorsToIgnore = {Owner};
	TArray<AActor*> OverlappingActors;

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		Owner->GetActorLocation(),
		SkillData->Range,
		TArray<TEnumAsByte<EObjectTypeQuery>>{UEngineTypes::ConvertToObjectType(ECC_Pawn)},
		nullptr,
		ActorsToIgnore,
		OverlappingActors);

	AActor* ClosestTarget = nullptr;
	float ClosestDistSq = FLT_MAX;
	const FVector OwnerLocation = Owner->GetActorLocation();

	for (AActor* Candidate : OverlappingActors)
	{
		if (!IsValid(Candidate))
		{
			continue;
		}

		// Section 29: filter by "Has EffectReceiverComponent", never by class.
		if (!IsValid(Candidate->FindComponentByClass<UEffectReceiverComponent>()))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestTarget = Candidate;
		}
	}

	return ClosestTarget;
}

void UCatSkillComponent::SpawnProjectile(AActor* TargetActor)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(SkillData))
	{
		return;
	}

	if (!IsValid(SkillData->ProjectileClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] SpawnProjectile skipped — SkillData->ProjectileClass "
			"unset on %s."), *Owner->GetName());
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Cast<APawn>(Owner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector SpawnLocation = Owner->GetActorLocation();
	const FRotator SpawnRotation = IsValid(TargetActor)
		? (TargetActor->GetActorLocation() - SpawnLocation).Rotation()
		: Owner->GetActorRotation();

	ACatPawProjectile* Projectile = World->SpawnActor<ACatPawProjectile>(
		SkillData->ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (!IsValid(Projectile))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] Failed to spawn projectile for %s."), *Owner->GetName());
		return;
	}

	Projectile->InitializeProjectile(TargetActor, BuildEffectSpecs(TargetActor));
	OnProjectileSpawned(Projectile);
}

TArray<FGameplayEffectSpec> UCatSkillComponent::BuildEffectSpecs(AActor* TargetActor) const
{
	TArray<FGameplayEffectSpec> Specs;
	if (!IsValid(SkillData))
	{
		return Specs;
	}

	Specs = SkillData->Effects;

	// NEXT.md section 6: repeated hits must escalate the actual payoff, not just the FX — each
	// GoldCost effect's Magnitude scales by how many times this target will have been stunned
	// once this hit lands (1st hit = base amount, 2nd = 2x, ... capped at MaxStunStack). Computed
	// here (before the hit happens) rather than in EffectReceiverComponent's dispatch so the
	// result never depends on which order Stun/GoldCost appear in SkillData->Effects[] — the skill
	// is the one place that already knows both the target and the Data Asset's base Magnitude.
	const UStatusComponent* TargetStatus = IsValid(TargetActor) ? TargetActor->FindComponentByClass<UStatusComponent>() : nullptr;
	const int32 UpcomingStunStack = IsValid(TargetStatus)
		? FMath::Clamp(TargetStatus->GetStunStack() + 1, 1, TargetStatus->GetMaxStunStack())
		: 1;

	for (FGameplayEffectSpec& Spec : Specs)
	{
		Spec.Source = GetOwner();
		Spec.Target = TargetActor;

		if (Spec.Type == CatGameplayTags::Effect_GoldCost)
		{
			Spec.Magnitude *= static_cast<float>(UpcomingStunStack);
		}
	}
	return Specs;
}

void UCatSkillComponent::SpawnAura(AActor* TargetActor)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	if (!IsValid(SkillData) || !IsValid(SkillData->AuraSystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] SpawnAura skipped — SkillData or AuraSystem missing on %s."),
			*Owner->GetName());
		return;
	}

	USceneComponent* AttachTarget = Owner->GetRootComponent();
	if (!IsValid(AttachTarget))
	{
		return;
	}

	ActiveAuraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SkillData->AuraSystem,
		AttachTarget,
		SkillData->AuraAttachSocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		/*bAutoDestroy=*/true);

	if (!IsValid(ActiveAuraComponent))
	{
		return;
	}

	// StunStack of the current target (0 if no target found yet, or the target has no
	// StatusComponent) — see NEXT.md section 7's Aura state mapping. MaxStunStack is read off the
	// target's own StatusComponent instance (it's an EditDefaultsOnly tunable now, not a shared
	// constant — a boss could have a different cap), so with no target we can't normalize against
	// anything meaningful and just report zero intensity instead of dividing by a guessed value.
	const UStatusComponent* TargetStatus = IsValid(TargetActor) ? TargetActor->FindComponentByClass<UStatusComponent>() : nullptr;
	const int32 CurrentStunStack = IsValid(TargetStatus) ? TargetStatus->GetStunStack() : 0;
	const float Intensity = IsValid(TargetStatus)
		? static_cast<float>(CurrentStunStack) / static_cast<float>(FMath::Max(TargetStatus->GetMaxStunStack(), 1))
		: 0.0f;

	ActiveAuraComponent->SetVariableInt(CatFXParamNames::StunStack, CurrentStunStack);
	ActiveAuraComponent->SetVariableFloat(CatFXParamNames::FXIntensity, Intensity);
}
