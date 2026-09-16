import unreal
import traceback

# ============================================================
# ONE-TIME FIX — force IMC_Default's E-key mapping to point at the
# EXACT SAME IA_CatSkill object that BP_PlayerCatSkill.CatSkillAction
# now references. If a stray duplicate IA_CatSkill exists somewhere,
# this makes pressing E fire the one BindAction is actually listening
# to, regardless of what IMC_Default was mapped to before.
# ============================================================

IMC_PATH = "/Game/Input/IMC_Default"
IA_PATH = "/Game/CatSkill/Input/IA_CatSkill"


def log(message):
    unreal.log(f"[CatSkillFixMapping] {message}")


def warn(message):
    unreal.log_warning(f"[CatSkillFixMapping] {message}")


if not unreal.EditorAssetLibrary.does_asset_exist(IMC_PATH):
    warn(f"{IMC_PATH} not found — edit this script's IMC_PATH to wherever "
        f"BP_PlayerCatSkill.DefaultMappingContext actually points.")
elif not unreal.EditorAssetLibrary.does_asset_exist(IA_PATH):
    warn(f"{IA_PATH} not found.")
else:
    imc = unreal.load_asset(IMC_PATH)
    ia_action = unreal.load_asset(IA_PATH)

    try:
        mappings = imc.get_editor_property("mappings")
        log(f"{IMC_PATH} currently has {len(mappings)} mapping(s):")
        for m in mappings:
            action = m.get_editor_property("action")
            key = m.get_editor_property("key")
            log(f"  - Action={action} Key={key}")
    except Exception as exc:
        warn(f"Could not read current mappings: {exc}")

    try:
        mappings = list(imc.get_editor_property("mappings"))
        log("Step 1 OK: read mappings list")
    except Exception:
        warn(f"Step 1 FAILED (get_editor_property mappings):\n{traceback.format_exc()}")
        mappings = None

    new_mapping = None
    if mappings is not None:
        try:
            new_mapping = unreal.EnhancedActionKeyMapping()
            log("Step 2 OK: constructed EnhancedActionKeyMapping")
        except Exception:
            warn(f"Step 2 FAILED (construct EnhancedActionKeyMapping):\n{traceback.format_exc()}")

    if new_mapping is not None:
        try:
            new_mapping.set_editor_property("action", ia_action)
            log("Step 3 OK: set action")
        except Exception:
            warn(f"Step 3 FAILED (set action):\n{traceback.format_exc()}")

        try:
            new_key = unreal.Key(key_name="E")
            log(f"Step 4a OK: constructed Key -> {new_key}")
        except Exception:
            warn(f"Step 4a FAILED (construct Key):\n{traceback.format_exc()}")
            new_key = None

        if new_key is not None:
            try:
                new_mapping.set_editor_property("key", new_key)
                log("Step 4b OK: set key")
            except Exception:
                warn(f"Step 4b FAILED (set key):\n{traceback.format_exc()}")

        try:
            mappings.append(new_mapping)
            imc.set_editor_property("mappings", mappings)
            log(f"Step 5 OK: appended mapping {IA_PATH} -> E inside {IMC_PATH}")
        except Exception:
            warn(f"Step 5 FAILED (append + set mappings):\n{traceback.format_exc()}")

    unreal.EditorAssetLibrary.save_loaded_asset(imc, only_if_is_dirty=False)
    log(f"Saved {IMC_PATH}.")

    try:
        mappings = imc.get_editor_property("mappings")
        log(f"{IMC_PATH} now has {len(mappings)} mapping(s):")
        for m in mappings:
            action = m.get_editor_property("action")
            key = m.get_editor_property("key")
            log(f"  - Action={action} Key={key}")
    except Exception as exc:
        warn(f"Could not re-read mappings after save: {exc}")

log("SAFE SCRIPT FINISHED.")
