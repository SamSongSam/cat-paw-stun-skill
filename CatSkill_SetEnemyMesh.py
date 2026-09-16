import unreal

# ============================================================
# ONE-TIME SETUP — give BP_EnemyTest a visible mannequin mesh
# Run once after CatSkill_Phase3Setup.py, from Tools -> Execute Python
# Script. Safe to re-run: just re-applies the same mesh/anim class.
# ============================================================

ENEMY_BP_PATH = "/Game/CatSkill/BP_EnemyTest"

# Quinn, not Manny — keeps the tester visually distinct from the player
# (which was set up earlier by copying the ThirdPerson template's own
# Manny-based components onto BP_PlayerCatSkill).
MESH_PATH = "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"
ANIM_BP_PATH = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"


def log(message):
    unreal.log(f"[CatSkillEnemyMesh] {message}")


def warn(message):
    unreal.log_warning(f"[CatSkillEnemyMesh] {message}")


if not unreal.EditorAssetLibrary.does_asset_exist(ENEMY_BP_PATH):
    warn(f"{ENEMY_BP_PATH} not found — run CatSkill_Phase3Setup.py first.")
else:
    bp = unreal.load_asset(ENEMY_BP_PATH)
    generated_class = bp.generated_class()
    cdo = unreal.get_default_object(generated_class)

    try:
        mesh_component = cdo.get_editor_property("mesh")
    except Exception as exc:
        mesh_component = None
        warn(f"Could not access Character's Mesh component: {exc}")

    if mesh_component is None:
        warn(f"{ENEMY_BP_PATH} has no accessible Mesh component — set the mesh manually in the Blueprint.")
    else:
        skeletal_mesh = unreal.load_asset(MESH_PATH)
        if skeletal_mesh is None:
            warn(f"Could not load {MESH_PATH} — check the path.")
        else:
            # UE renamed SkeletalMeshComponent's property across versions —
            # try both rather than guessing this engine's exact name.
            set_ok = False
            for prop_name in ("skeletal_mesh_asset", "skeletal_mesh"):
                try:
                    mesh_component.set_editor_property(prop_name, skeletal_mesh)
                    set_ok = True
                    log(f"Set Mesh.{prop_name} -> {MESH_PATH}")
                    break
                except Exception:
                    continue
            if not set_ok:
                warn(f"Could not set the skeletal mesh via either known property name — "
                    f"assign {MESH_PATH} to the Mesh component manually.")

        anim_class = unreal.load_class(None, ANIM_BP_PATH)
        if anim_class is None:
            warn(f"Could not load anim class {ANIM_BP_PATH} — check the path, "
                f"or assign an Animation Blueprint to the Mesh component manually.")
        else:
            try:
                mesh_component.set_editor_property("anim_class", anim_class)
                log(f"Set Mesh.AnimClass -> {ANIM_BP_PATH}")
            except Exception as exc:
                warn(f"Could not set Mesh.AnimClass: {exc}")

    try:
        # Character's capsule is a common size mismatch source with Quinn/Manny —
        # nudge the mesh down/rotated to the standard mannequin offset used by
        # the ThirdPerson template, matching what BP_PlayerCatSkill already has.
        mesh_component.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, -90.0))
        mesh_component.set_editor_property("relative_rotation", unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0))
    except Exception as exc:
        warn(f"Could not set Mesh relative transform: {exc} — adjust it by eye in the Blueprint viewport.")

    unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
    log(f"Saved {ENEMY_BP_PATH}.")

log("SAFE SCRIPT FINISHED.")
