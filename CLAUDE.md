# Cat Paw Stun Skill — Unreal Engine 5 Project

Gameplay VFX skill: cat character presses E → pink aura → procedural (SDF, no sprite sheet) paw projectiles dash to target → stun → enemy pays gold → cat receives a treat and licks the owner. Full design doc: [CatSkill_test.md](CatSkill_test.md).

Architecture reference (from CatSkill_test.md section 40, the project's Golden Rule):

```
Gameplay?       → C++
Data?           → Data Asset
Presentation?   → Blueprint
FX?             → Niagara
Shape/Surface?  → Material / Shader
Character Motion? → Animation / Control Rig
AI behavior?    → AI / Interaction Component
```

Keep it this way — do not let gameplay logic leak into Blueprint, and do not let C++ own animation/sound/camera timing.

## Current status

The full C++ gameplay chain (Input → Aura → Target → Projectile → Hit → Stun/Gold/CarryItem dispatch) is implemented under [Source/MyProject/](Source/MyProject/), plus the Dynamic FX parameter contract and a gold-cost escalation tied to the stun stack:
- `Gameplay/Skills/CatSkillData.h` / `CatSkillComponent.h/.cpp` — `ActivateCatSkill()`: cooldown check, target search (sphere overlap), spawn aura (with current StunStack pushed as a Niagara param), spawn projectile, build effect specs (GoldCost Magnitude scaled by the target's upcoming StunStack)
- `Gameplay/Player/CatPlayerCharacter.h/.cpp` — Enhanced Input wiring (no Tick polling)
- `Gameplay/Projectiles/CatPawProjectile.h/.cpp` — homing dash, on-hit effect delivery, pushes Distance/Velocity/Hit Niagara params
- `Gameplay/Status/StatusComponent.h/.cpp` — Stun + StunStack (0..`MaxStunStack`, `EditDefaultsOnly`); the stack auto-resets when a stun that maxed it naturally wears off (see file for why the reset isn't instant)
- `Gameplay/Effects/GameplayEffectTypes.h`, `EffectReceiverComponent.h/.cpp`, `CatGameplayTags.h/.cpp` — effect dispatch, now keyed by `FGameplayTag` (Editor-authored, not a C++ enum — see `CatGameplayTags.h`)
- `Gameplay/Effects/CatFXParamNames.h` — shared Niagara User Parameter name constants
- `Gameplay/Economy/EconomyComponent.h/.cpp`, `Gameplay/Equipment/`, `Gameplay/Interaction/`, `Gameplay/Animation/` — gold wallet, carry-item attach, AI delivery interaction, lick/treat anim notifies (present but not yet exercised end-to-end)
- [Source/MyProject/INTEGRATION_NOTES.md](Source/MyProject/INTEGRATION_NOTES.md) — module deps to add to Build.cs, editor assets that must be created by hand, the cooldown formula, the Niagara User Parameters each system needs authored, and how to add new effect-type tags

**Not yet implemented**: procedural SDF paw shader/material (Phase 12 — this is Editor Material Graph work, not C++), shape-swap system, the Cat Treat/Lick payoff's actual AI+animation sequence (the `StatusComponent`/economy hooks it will trigger from already exist), and the Niagara graphs themselves for the Dynamic FX parameter contract (the C++ pushes the values; nothing yet consumes them visually — see INTEGRATION_NOTES.md section 7).

**This code has not been compiled or run** — it was written outside an actual `.uproject`, to be copied into one. See INTEGRATION_NOTES.md before expecting it to build. Do not claim a phase is "done" without an actual PIE test confirming it — see rule 5 below.

## Automation scripts (run manually inside the UE Python console — not run by Claude)

- [CatAura_SafeBuild.py](CatAura_SafeBuild.py) — duplicates a known-good Niagara System template into `NS_CatAura`, creates the master material + instances. Requires selecting a working Niagara System in Content Browser first.
- [CatSkill_AutoWire.py](CatSkill_AutoWire.py) — creates/wires `IMC_Default`, `IA_CatSkill`, `DA_CatPawSkill`, `BP_PlayerCatSkill` and links them together, once the C++ classes above are compiled into the project. Run `CatAura_SafeBuild.py` first.

Both scripts are conservative by design: they skip existing assets unless `OVERWRITE_EXISTING = True`, never delete source assets, and log warnings instead of crashing when something is missing — keep new automation scripts in this style.

## Code style (mandatory — carried over from the shared FX-pipeline rules)

1. **OOP always** — no loose procedural scripts/functions.
2. **Documentation is mandatory, not optional polish, on every `.h`/`.cpp` in `Source/MyProject/`, with no exceptions for "small" files:**
   - **File header**: exactly one `// #region SUMMARY` / `// #endregion` block at the top of the file — Purpose / Depends on / Consumed by / Exposed params. This is the only region pair that spans the whole file.
   - **Section regions inside the class body**: group related members under their own `// #region <name>` / `// #endregion` pairs — e.g. `Public API`, a named pipeline/flow region for ordered internal calls, `Tunables and runtime state`, `Presentation hooks`. Don't leave a class as one undivided block once it has more than a couple of members — a reader (or you, six months later, hunting a bug) should be able to jump straight to "the part that matters" from the region names alone.
   - **Inline comments on the actual logic, not just the function signature**: every non-trivial branch, guard clause, loop, or ordering decision gets a comment explaining *why*, not what (the code already says what). A function with only a one-line comment above its signature and no comments in its body is not documented — see `StatusComponent.cpp`/`CatPawProjectile.cpp` for the expected density.
   - This applies retroactively — if you touch a file that predates this rule and it's missing regions/comments, bring it up to standard as part of that change, don't just add to the gap.
3. **Every tunable value auto-exposes to the editor** — `UPROPERTY(EditAnywhere/EditDefaultsOnly, BlueprintReadWrite/ReadOnly)`, never hardcode a value a designer should be able to tune (this includes gameplay-balance constants like stack ceilings, not just VFX numbers — see `UStatusComponent::MaxStunStack`). Reusable logic goes on a Data Asset, not hardcoded in a component. Closed C++ enums are also a form of hardcoding when the set of values is something a designer should be able to extend — use `FGameplayTag` instead (see `GameplayEffectTypes.h` / `CatGameplayTags.h`) so new values can be added from the Editor without a recompile.
4. **Crash-prevention checklist before every compile/PR** (see the original FX-pipeline unreal notes): `GENERATED_BODY()` + `UCLASS()` present, `IsValid()`/`nullptr` check every `UPROPERTY` object reference before dereferencing, never change a `UCLASS`/`USTRUCT` layout and hot-reload mid-PIE, test in PIE before calling something done, attach crash logs from `Saved/Logs/` or `Saved/Crashes/` when reporting a crash — not just "กดแล้วเด้ง".
5. **No premature "done"** — code that hasn't been compiled/run in an actual `.uproject` + PIE session is "implemented, unverified," never "working" or "complete." Say which one it is.

## Module setup still required

`MYPROJECT_API` in every header is a placeholder — replace with the real module macro once copied into an actual `.uproject`, and add `EnhancedInput` + `Niagara` to that module's `Build.cs` `PublicDependencyModuleNames`. Full details in [Source/MyProject/INTEGRATION_NOTES.md](Source/MyProject/INTEGRATION_NOTES.md).
