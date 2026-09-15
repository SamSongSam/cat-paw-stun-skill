#pragma once
// #region SUMMARY
// File: CatPlayerCharacter.h
// Purpose: Phase 1 slice — C++ base for the player pawn. Adds the Enhanced Input mapping
//          context on BeginPlay and binds IA_CatSkill (Started) to CatSkillComponent. This is
//          the C++/Blueprint bridge point: BP_PlayerCatSkill (Blueprint) derives from this class
//          and adds mesh/camera/animation (presentation) only — see CatSkill_test.md section 33.
// Depends on: UCatSkillComponent (CatSkillComponent.h); EnhancedInput module (UInputMappingContext,
//             UInputAction) — must be added to <Module>.Build.cs, see INTEGRATION_NOTES.md
// Consumed by: BP_PlayerCatSkill (Blueprint subclass, created in the editor, not here)
// Exposed params: DefaultMappingContext, CatSkillAction — assign IMC_Default / IA_CatSkill on the
//                 Blueprint's Class Defaults; CatSkillComponent's own SkillData is set there too
// #endregion

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CatPlayerCharacter.generated.h"

class UCatSkillComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class MYPROJECT_API ACatPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACatPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void HandleCatSkillInput(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat Skill")
	TObjectPtr<UCatSkillComponent> CatSkillComponent;

	// Assigned per-Blueprint in Class Defaults — never hardcode a specific IMC/IA asset path in C++.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CatSkillAction;
};
