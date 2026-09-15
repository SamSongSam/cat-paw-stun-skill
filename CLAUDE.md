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

**Phase 1-2 only** (Input → Aura) is implemented in C++, under [Source/MyProject/](Source/MyProject/):
- `Gameplay/Skills/CatSkillData.h` — Data Asset (SkillDisplayName, Cooldown, AuraSystem, AuraAttachSocketName)
- `Gameplay/Skills/CatSkillComponent.h/.cpp` — `ActivateCatSkill()`, cooldown check, spawn aura, `OnCatSkillActivated()` Blueprint event
- `Gameplay/Player/CatPlayerCharacter.h/.cpp` — Enhanced Input wiring (no Tick polling)
- [Source/MyProject/INTEGRATION_NOTES.md](Source/MyProject/INTEGRATION_NOTES.md) — module deps to add to Build.cs, editor assets that must be created by hand, the cooldown formula

**Not yet implemented** (per the doc's own vertical-slice build order, section 35): target detection, projectile, hit/stun, economy (gold), carry item, AI delivery, cat lick, procedural paw shader, shape-swap system. Do not skip ahead to these without finishing/testing Phase 1-2 first.

**This code has not been compiled or run** — it was written outside an actual `.uproject`, to be copied into one. See INTEGRATION_NOTES.md before expecting it to build.

## Automation scripts (run manually inside the UE Python console — not run by Claude)

- [CatAura_SafeBuild.py](CatAura_SafeBuild.py) — duplicates a known-good Niagara System template into `NS_CatAura`, creates the master material + instances. Requires selecting a working Niagara System in Content Browser first.
- [CatSkill_AutoWire.py](CatSkill_AutoWire.py) — creates/wires `IMC_Default`, `IA_CatSkill`, `DA_CatPawSkill`, `BP_PlayerCatSkill` and links them together, once the C++ classes above are compiled into the project. Run `CatAura_SafeBuild.py` first.

Both scripts are conservative by design: they skip existing assets unless `OVERWRITE_EXISTING = True`, never delete source assets, and log warnings instead of crashing when something is missing — keep new automation scripts in this style.

## Code style (mandatory — carried over from the shared FX-pipeline rules)

1. **OOP always** — no loose procedural scripts/functions.
2. **`// #region SUMMARY` / `// #endregion` block at the top of every file** — Purpose / Depends on / Consumed by / Exposed params. Every `.h`/`.cpp` in `Source/MyProject/` already follows this; keep doing it for every new file.
3. **Every tunable value auto-exposes to the editor** — `UPROPERTY(EditAnywhere/EditDefaultsOnly, BlueprintReadWrite/ReadOnly)`, never hardcode a value a designer should be able to tune. Reusable logic goes on a Data Asset, not hardcoded in a component.
4. **Crash-prevention checklist before every compile/PR** (see the original FX-pipeline unreal notes): `GENERATED_BODY()` + `UCLASS()` present, `IsValid()`/`nullptr` check every `UPROPERTY` object reference before dereferencing, never change a `UCLASS`/`USTRUCT` layout and hot-reload mid-PIE, test in PIE before calling something done, attach crash logs from `Saved/Logs/` or `Saved/Crashes/` when reporting a crash — not just "กดแล้วเด้ง".

## Module setup still required

`MYPROJECT_API` in every header is a placeholder — replace with the real module macro once copied into an actual `.uproject`, and add `EnhancedInput` + `Niagara` to that module's `Build.cs` `PublicDependencyModuleNames`. Full details in [Source/MyProject/INTEGRATION_NOTES.md](Source/MyProject/INTEGRATION_NOTES.md).
