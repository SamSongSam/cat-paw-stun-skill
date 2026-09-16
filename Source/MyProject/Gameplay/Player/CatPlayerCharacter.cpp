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
#include "InputMappingContext.h"
#include "Engine/Engine.h"

// Temporary debug helper — on-screen messages so wiring issues are visible
// immediately in PIE instead of needing a log-file round trip each time.
// Remove once Phase 1-2 input is confirmed working end to end.
static void CatSkillDebugMessage(const FString& Message, FColor Color = FColor::Yellow)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.0f, Color, TEXT("[CatSkillDebug] ") + Message);
	}
}

ACatPlayerCharacter::ACatPlayerCharacter()
{
	CatSkillComponent = CreateDefaultSubobject<UCatSkillComponent>(TEXT("CatSkillComponent"));

	// This class is meant to be hand-placed in the level as THE player
	// character (not spawned by GameMode at a PlayerStart) — auto-possess it
	// as Player 0 by default so dragging BP_PlayerCatSkill into a level and
	// hitting Play just works, without per-instance Blueprint setup.
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ACatPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!IsValid(PC))
	{
		CatSkillDebugMessage(TEXT("BeginPlay: GetController() is NOT a PlayerController."), FColor::Red);
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		CatSkillDebugMessage(TEXT("BeginPlay: PlayerController has no LocalPlayer."), FColor::Red);
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(Subsystem))
	{
		CatSkillDebugMessage(TEXT("BeginPlay: No EnhancedInputLocalPlayerSubsystem."), FColor::Red);
		return;
	}

	if (!IsValid(DefaultMappingContext))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] %s has no DefaultMappingContext assigned."), *GetName());
		CatSkillDebugMessage(TEXT("BeginPlay: DefaultMappingContext is NOT assigned."), FColor::Red);
		return;
	}

	Subsystem->AddMappingContext(DefaultMappingContext, /*Priority=*/0);
	CatSkillDebugMessage(FString::Printf(TEXT("BeginPlay: Added mapping context '%s'."),
		*DefaultMappingContext->GetName()), FColor::Green);
}

void ACatPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(EnhancedInput))
	{
		UE_LOG(LogTemp, Error, TEXT("[CatSkill] PlayerInputComponent is not an EnhancedInputComponent — "
			"check Project Settings > Input > Default Classes."));
		CatSkillDebugMessage(TEXT("SetupPlayerInputComponent: NOT an EnhancedInputComponent!"), FColor::Red);
		return;
	}

	if (!IsValid(CatSkillAction))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CatSkill] %s has no CatSkillAction assigned."), *GetName());
		CatSkillDebugMessage(TEXT("SetupPlayerInputComponent: CatSkillAction is NOT assigned."), FColor::Red);
		return;
	}

	EnhancedInput->BindAction(CatSkillAction, ETriggerEvent::Started, this,
		&ACatPlayerCharacter::HandleCatSkillInput);
	CatSkillDebugMessage(FString::Printf(TEXT("SetupPlayerInputComponent: Bound '%s' to HandleCatSkillInput."),
		*CatSkillAction->GetName()), FColor::Green);
}

void ACatPlayerCharacter::HandleCatSkillInput(const FInputActionValue& Value)
{
	CatSkillDebugMessage(TEXT("HandleCatSkillInput: E RECEIVED."), FColor::Cyan);

	if (IsValid(CatSkillComponent))
	{
		CatSkillComponent->ActivateCatSkill();
	}
	else
	{
		CatSkillDebugMessage(TEXT("HandleCatSkillInput: CatSkillComponent is invalid!"), FColor::Red);
	}
}
