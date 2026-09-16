import unreal

# ============================================================
# ONE-TIME FIX — repoint BP_PlayerCatSkill.DefaultMappingContext from
# IMC_Default to IMC_Cat (the user manually built IMC_Cat with the E ->
# IA_CatSkill mapping in the Editor UI, since the Python "mappings"
# property is deprecated/broken in this engine build — see
# CatSkill_FixInputMapping.py's debug trail for why).
# Run once from Tools -> Execute Python Script.
# ============================================================

IMC_CAT_PATH = "/Game/CatSkill/Input/IMC_Cat"
PLAYER_BP_PATH = "/Game/CatSkill/BP_PlayerCatSkill"
FALLBACK_PLAYER_BP_PATH = "/Game/Blueprints/Characters/BP_PlayerCatSkill"


def log(message):
    unreal.log(f"[CatSkillSwitchIMC] {message}")


def warn(message):
    unreal.log_warning(f"[CatSkillSwitchIMC] {message}")


if not unreal.EditorAssetLibrary.does_asset_exist(IMC_CAT_PATH):
    warn(f"{IMC_CAT_PATH} not found.")
else:
    imc_cat = unreal.load_asset(IMC_CAT_PATH)

    bp = None
    bp_path = None
    for path in (PLAYER_BP_PATH, FALLBACK_PLAYER_BP_PATH):
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            bp = unreal.load_asset(path)
            bp_path = path
            break

    if bp is None:
        warn(f"BP_PlayerCatSkill not found at {PLAYER_BP_PATH} or {FALLBACK_PLAYER_BP_PATH}.")
    else:
        generated_class = bp.generated_class()
        cdo = unreal.get_default_object(generated_class)

        try:
            cdo.set_editor_property("default_mapping_context", imc_cat)
            log(f"Set {bp_path}.DefaultMappingContext -> {IMC_CAT_PATH}")
        except Exception as exc:
            warn(f"Could not set DefaultMappingContext: {exc}")

        unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
        log(f"Saved {bp_path}.")

log("SAFE SCRIPT FINISHED.")
