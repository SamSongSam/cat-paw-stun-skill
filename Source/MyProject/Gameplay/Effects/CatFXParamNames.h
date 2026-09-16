#pragma once
// #region SUMMARY
// File: CatFXParamNames.h
// Purpose: Single source of truth for the Niagara User Parameter names the C++ gameplay layer
//          writes to (NEXT.md's C++ <-> Niagara Dynamic FX parameter contract). Keeps the FName
//          strings out of CatSkillComponent.cpp / CatPawProjectile.cpp so both stay in sync with
//          the actual User Parameters authored on the Niagara Systems (a typo here silently no-ops
//          in Niagara instead of failing to compile).
// Depends on: none (leaf constants header)
// Consumed by: CatSkillComponent.cpp (aura params), CatPawProjectile.cpp (trail + impact params)
// Exposed params: n/a — these are Niagara User Parameter names, not UPROPERTYs; the actual tunable
//                 values live on the Niagara Systems themselves (artist-facing, per project Golden Rule)
// #endregion

#include "CoreMinimal.h"

// Must match the User Parameter names authored on NS_CatAura, the paw projectile's own Niagara
// system, and its ImpactSystem — see INTEGRATION_NOTES.md section 6 for the full per-asset list.
namespace CatFXParamNames
{
	static const FName StunStack(TEXT("StunStack"));
	static const FName FXIntensity(TEXT("FXIntensity"));
	static const FName TargetPosition(TEXT("TargetPosition"));
	static const FName DistanceToTarget(TEXT("DistanceToTarget"));
	static const FName AttackVelocity(TEXT("AttackVelocity"));
	static const FName HitLocation(TEXT("HitLocation"));
	static const FName HitNormal(TEXT("HitNormal"));
}
