import unreal

# ============================================================
# ONE-TIME SETUP — give BP_PlayerCatSkill a visible mesh + a camera
# Run once from Tools -> Execute Python Script. Safe to re-run: skips
# adding CameraBoom/FollowCamera if they already exist (e.g. from the
# earlier manual Ctrl+C/Ctrl+V component copy from BP_ThirdPersonCharacter).
#
# ACatPlayerCharacter (C++) is a bare Character on purpose (Golden Rule:
# gameplay owns no presentation) — it already has a Mesh component
# (inherited from ACharacter), but no SpringArm/Camera, so without this
# (or the manual copy) Play would show nothing: no visible mesh look and
# no camera to view from.
# ============================================================

PLAYER_BP_PATH = "/Game/CatSkill/BP_PlayerCatSkill"
FALLBACK_PLAYER_BP_PATH = "/Game/Blueprints/Characters/BP_PlayerCatSkill"

MESH_PATH = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"
ANIM_BP_PATH = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"


def log(message):
    unreal.log(f"[CatSkillPlayerVisuals] {message}")


def warn(message):
    unreal.log_warning(f"[CatSkillPlayerVisuals] {message}")


def find_player_bp():
    for path in (PLAYER_BP_PATH, FALLBACK_PLAYER_BP_PATH):
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            return unreal.load_asset(path), path
    return None, None


def get_subobject_handle_by_name(subsystem, bp, name):
    for handle in subsystem.k2_gather_subobject_data_for_blueprint(bp):
        try:
            obj = subsystem.get_object(handle)
            if obj is not None and obj.get_name() == name:
                return handle
        except Exception:
            continue
    return None


def add_scene_component(subsystem, bp, parent_handle, component_class, name):
    if get_subobject_handle_by_name(subsystem, bp, name) is not None:
        log(f"{name} already exists on {bp.get_name()} — skipping.")
        return get_subobject_handle_by_name(subsystem, bp, name)

    params = unreal.AddNewSubobjectParams(
        parent_handle=parent_handle,
        new_class=component_class,
        blueprint_context=bp,
    )
    new_handle, fail_reason = subsystem.add_new_subobject(params)
    if fail_reason and str(fail_reason):
        warn(f"add_new_subobject({name}) reported: {fail_reason}")
    subsystem.rename_subobject(handle=new_handle, new_name=unreal.Text(name))
    log(f"Added {name} to {bp.get_name()}.")
    return new_handle


bp, bp_path = find_player_bp()
if bp is None:
    warn(f"Neither {PLAYER_BP_PATH} nor {FALLBACK_PLAYER_BP_PATH} exist — "
        f"check where BP_PlayerCatSkill actually is and edit this script's path.")
else:
    generated_class = bp.generated_class()
    cdo = unreal.get_default_object(generated_class)

    # --- Mesh + Animation ---
    try:
        mesh_component = cdo.get_editor_property("mesh")
    except Exception as exc:
        mesh_component = None
        warn(f"Could not access Character's Mesh component: {exc}")

    if mesh_component is not None:
        skeletal_mesh = unreal.load_asset(MESH_PATH)
        if skeletal_mesh is not None:
            for prop_name in ("skeletal_mesh_asset", "skeletal_mesh"):
                try:
                    mesh_component.set_editor_property(prop_name, skeletal_mesh)
                    log(f"Set Mesh.{prop_name} -> {MESH_PATH}")
                    break
                except Exception:
                    continue
        else:
            warn(f"Could not load {MESH_PATH}.")

        anim_class = unreal.load_class(None, ANIM_BP_PATH)
        if anim_class is not None:
            try:
                mesh_component.set_editor_property("anim_class", anim_class)
                log(f"Set Mesh.AnimClass -> {ANIM_BP_PATH}")
            except Exception as exc:
                warn(f"Could not set Mesh.AnimClass: {exc}")

        try:
            mesh_component.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, -90.0))
            mesh_component.set_editor_property("relative_rotation", unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0))
        except Exception as exc:
            warn(f"Could not set Mesh relative transform: {exc}")
    else:
        warn(f"{bp_path} has no accessible Mesh component — set the mesh manually.")

    # --- Camera (SpringArm + Camera), only if not already present ---
    try:
        subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        root_handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
        if not root_handles:
            warn(f"{bp_path}: no root subobject handle — cannot add CameraBoom/FollowCamera.")
        else:
            spring_arm_class = getattr(unreal, "SpringArmComponent", None)
            camera_class = getattr(unreal, "CameraComponent", None)

            if spring_arm_class is None or camera_class is None:
                warn("unreal.SpringArmComponent or unreal.CameraComponent not found — "
                    "add CameraBoom/FollowCamera manually.")
            else:
                boom_handle = add_scene_component(subsystem, bp, root_handles[0], spring_arm_class, "CameraBoom")
                if boom_handle is not None:
                    boom = subsystem.get_object(boom_handle)
                    try:
                        boom.set_editor_property("target_arm_length", 400.0)
                        boom.set_editor_property("use_pawn_control_rotation", True)
                    except Exception as exc:
                        warn(f"Could not set CameraBoom properties: {exc}")

                    add_scene_component(subsystem, bp, boom_handle, camera_class, "FollowCamera")
    except Exception as exc:
        warn(f"Could not add CameraBoom/FollowCamera via script: {exc} — "
            f"copy them from BP_ThirdPersonCharacter's Components panel (Ctrl+C/Ctrl+V) instead.")

    unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
    log(f"Saved {bp_path}.")

log("SAFE SCRIPT FINISHED.")
