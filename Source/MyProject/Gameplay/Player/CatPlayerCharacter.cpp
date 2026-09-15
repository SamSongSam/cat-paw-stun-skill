// #region SUMMARY
// File: CatPlayerCharacter.cpp
// Purpose: Implements Phase 1 input wiring — add mapping context, bind IA_CatSkill to
//          CatSkillComponent->ActivateCatSkill(). No Tick-based input polling (per
//          CatSkill_test.md section 4: "ไม่ใช้ Tick เช็ก E").
// Depends on: CatPlayerCharacter.h, CatSkillComponent.h, EnhancedInputSubsystems (engine)
// Consumed by: n/a (module compile unit; BP_PlayerCatSkill derives from this class in-editor)
// Exposed params: none beyond the header's UPROPERTY list
// #endregion

#include "CatPlayerCharacter.h"
#include "CatSkillComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ACatPlayerCharacter::ACatPlayerCharacter()
{
	CatSkillComponent = CreateDefaultSubobject<UCatSkillComponent>(TEXT("CatSkillComponent"));
}

void ACatPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!IsValid(PC))
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(Subsystem))
	{
		return;
	}

	if (!IsValid(DefaultMappingContext))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] %s has no DefaultMappingContext assigned."), *GetName());
		return;
	}

	Subsystem->AddMappingContext(DefaultMappingContext, /*Priority=*/0);
}

void ACatPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(EnhancedInput))
	{
		UE_LOG(LogTemp, Error, TEXT("[CatSkill] PlayerInputComponent is not an EnhancedInputComponent — "
			"check Project Settings > Input > Default Classes."));
		return;
	}

	if (!IsValid(CatSkillAction))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] %s has no CatSkillAction assigned."), *GetName());
		return;
	}

	EnhancedInput->BindAction(CatSkillAction, ETriggerEvent::Started, this,
		&ACatPlayerCharacter::HandleCatSkillInput);
}

void ACatPlayerCharacter::HandleCatSkillInput(const FInputActionValue& Value)
{
	if (IsValid(CatSkillComponent))
	{
		CatSkillComponent->ActivateCatSkill();
	}
}
