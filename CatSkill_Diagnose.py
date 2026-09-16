import unreal

# ============================================================
# DIAGNOSTIC ONLY — reads current state, changes nothing.
# Run from Tools -> Execute Python Script.
# ============================================================

def log(message):
    unreal.log(f"[CatSkillDiag] {message}")


# 1. Check for duplicate IA_CatSkill / BP_PlayerCatSkill anywhere in the project
ar = unreal.AssetRegistryHelpers.get_asset_registry()
all_assets = ar.get_all_assets()
for name in ("IA_CatSkill", "BP_PlayerCatSkill", "IMC_Cat", "IMC_Default"):
    matches = [a for a in all_assets if str(a.asset_name) == name]
    log(f"Search '{name}': {len(matches)} match(es)")
    for a in matches:
        log(f"  - {a.package_name}")

# 2. Read BP_PlayerCatSkill's CDO right now
BP_PATH = "/Game/CatSkill/BP_PlayerCatSkill"
if unreal.EditorAssetLibrary.does_asset_exist(BP_PATH):
    bp = unreal.load_asset(BP_PATH)
    generated_class = bp.generated_class()
    cdo = unreal.get_default_object(generated_class)
    try:
        action = cdo.get_editor_property("cat_skill_action")
        imc = cdo.get_editor_property("default_mapping_context")
        log(f"CDO right now: CatSkillAction={action}  DefaultMappingContext={imc}")
    except Exception as exc:
        log(f"Could not read CDO properties: {exc}")
else:
    log(f"{BP_PATH} does not exist.")

# 3. Check every level's World Settings GameMode / Default Pawn Class override
editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.EditorLevelLibrary.get_editor_world() if hasattr(unreal, "EditorLevelLibrary") else None
if world is None:
    try:
        world = unreal.UnrealEditorSubsystem().get_editor_world()
    except Exception:
        world = None

if world is not None:
    log(f"Current editor world: {world.get_name()}")
    world_settings = world.get_world_settings()
    try:
        gamemode_override = world_settings.get_editor_property("default_game_mode")
        log(f"World Settings GameMode Override: {gamemode_override}")
        if gamemode_override is not None:
            gm_cdo = unreal.get_default_object(gamemode_override)
            pawn_class = gm_cdo.get_editor_property("default_pawn_class")
            log(f"GameMode's DefaultPawnClass: {pawn_class}")
    except Exception as exc:
        log(f"Could not read World Settings GameMode: {exc}")
else:
    log("Could not get editor world.")

log("DIAGNOSTIC FINISHED.")
