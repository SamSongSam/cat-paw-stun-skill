import unreal

# ============================================================
# ONE-TIME FIX — BP_PlayerCatSkill.CatSkillAction was never set
# because IA_CatSkill didn't exist yet when CatSkill_AutoWire.py first
# ran (unreal.InputActionFactory isn't available in this engine build).
# Run once from Tools -> Execute Python Script.
# ============================================================

IA_PATH = "/Game/CatSkill/Input/IA_CatSkill"
PLAYER_BP_CANDIDATES = [
    "/Game/CatSkill/BP_PlayerCatSkill",
    "/Game/Blueprints/Characters/BP_PlayerCatSkill",
]


def log(message):
    unreal.log(f"[CatSkillFixInput] {message}")


def warn(message):
    unreal.log_warning(f"[CatSkillFixInput] {message}")


if not unreal.EditorAssetLibrary.does_asset_exist(IA_PATH):
    warn(f"{IA_PATH} not found — check where IA_CatSkill actually is and edit this script's path.")
else:
    ia_action = unreal.load_asset(IA_PATH)

    bp = None
    bp_path = None
    for path in PLAYER_BP_CANDIDATES:
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            bp = unreal.load_asset(path)
            bp_path = path
            break

    if bp is None:
        warn(f"BP_PlayerCatSkill not found at any of: {PLAYER_BP_CANDIDATES}")
    else:
        generated_class = bp.generated_class()
        cdo = unreal.get_default_object(generated_class)

        try:
            cdo.set_editor_property("cat_skill_action", ia_action)
            log(f"Set {bp_path}.CatSkillAction -> {IA_PATH}")
        except Exception as exc:
            warn(f"Could not set CatSkillAction: {exc}")

        try:
            current_imc = cdo.get_editor_property("default_mapping_context")
            log(f"DefaultMappingContext is currently: {current_imc}")
        except Exception as exc:
            warn(f"Could not read DefaultMappingContext: {exc}")

        unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
        log(f"Saved {bp_path}.")

log("SAFE SCRIPT FINISHED.")
