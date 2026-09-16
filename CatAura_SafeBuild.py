import unreal

# ============================================================
# CAT AURA — SAFE BUILD (Phase 2: pink aura VFX)
# Unreal Engine 5.x
#
# RUN ORDER (important):
#   1. Select a Niagara System that ALREADY WORKS in the Content Browser
#      (any existing, playable NS_* asset — this script duplicates it as
#      a starting point, it does not build an emitter from scratch).
#   2. Run this script.
#   3. Run CatSkill_AutoWire.py next — it links DA_CatPawSkill.AuraSystem
#      to the NS_CatAura this script creates.
#
# WHAT THIS SCRIPT DOES:
# - Duplicates the selected Niagara System to NS_CAT_AURA_PATH (skips if
#   NS_CatAura already exists, unless OVERWRITE_EXISTING=True).
# - Creates M_CatAura, a simple unlit translucent master material with an
#   Emissive Color (vector) parameter and an Opacity (scalar) parameter.
# - Creates MI_CatAura_Pink, a Material Instance of M_CatAura pre-tinted
#   pink (per CatSkill_test.md — pink aura on skill activation).
#
# WHAT THIS SCRIPT DOES NOT DO:
# - Does not touch or modify the source Niagara System you selected.
# - Does not wire the material into the duplicated Niagara System's
#   renderer — Niagara emitter/renderer editing is not scriptable in a
#   safe, version-stable way from Python; do that one step by hand in the
#   Niagara editor (open NS_CatAura -> Sprite/Ribbon Renderer -> assign
#   MI_CatAura_Pink as Material).
# - Does not delete or overwrite existing assets unless OVERWRITE_EXISTING=True.
# ============================================================


# ============================================================
# CONFIG — adjust paths to match your project
# ============================================================

NS_TARGET_DIR = "/Game/CatSkill"
NS_TARGET_NAME = "NS_CatAura"

MAT_DIR = "/Game/CatSkill/Materials"
MASTER_MAT_NAME = "M_CatAura"
INSTANCE_NAME = "MI_CatAura_Pink"

# Pink aura tint (linear color, RGBA) — see CatSkill_test.md: "pink aura".
AURA_TINT = unreal.LinearColor(1.0, 0.2, 0.75, 1.0)
AURA_OPACITY_DEFAULT = 0.6

# Keep False while testing — avoids clobbering hand-tuned assets.
OVERWRITE_EXISTING = False


# ============================================================
# UTILITIES (same pattern as CatSkill_AutoWire.py)
# ============================================================

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_lib = unreal.EditorAssetLibrary
mat_lib = unreal.MaterialEditingLibrary


def log(message):
    unreal.log(f"[CatAuraBuild] {message}")


def warn(message):
    unreal.log_warning(f"[CatAuraBuild] {message}")


def fail(message):
    unreal.log_error(f"[CatAuraBuild] {message}")
    raise RuntimeError(message)


def ensure_directory(path):
    if not editor_lib.does_directory_exist(path):
        editor_lib.make_directory(path)
        log(f"Created folder: {path}")


def asset_exists(path):
    return editor_lib.does_asset_exist(path)


def save_asset(asset):
    if asset:
        editor_lib.save_loaded_asset(asset, only_if_is_dirty=False)


for folder in [NS_TARGET_DIR, MAT_DIR]:
    ensure_directory(folder)


# ============================================================
# NIAGARA SYSTEM — duplicate the selected template
# ============================================================

def get_selected_niagara_system():
    selected = unreal.EditorUtilityLibrary.get_selected_assets()
    if not selected:
        fail(
            "Nothing selected in the Content Browser. Select a working "
            "Niagara System asset first, then re-run this script."
        )

    for asset in selected:
        if isinstance(asset, unreal.NiagaraSystem):
            return asset

    fail(
        "The current Content Browser selection has no Niagara System in "
        "it. Select an existing, working NS_* asset first."
    )


def build_aura_system():
    target_path = f"{NS_TARGET_DIR}/{NS_TARGET_NAME}"

    if asset_exists(target_path):
        if not OVERWRITE_EXISTING:
            log(f"{NS_TARGET_NAME} already exists — skipping duplicate.")
            return unreal.load_asset(target_path)
        editor_lib.delete_asset(target_path)
        warn(f"OVERWRITE_EXISTING=True — deleted existing {target_path}.")

    source = get_selected_niagara_system()
    source_path = source.get_path_name().split(".")[0]

    duplicated = asset_tools.duplicate_asset(NS_TARGET_NAME, NS_TARGET_DIR, source)
    if not duplicated:
        fail(f"Failed to duplicate {source_path} -> {target_path}.")

    save_asset(duplicated)
    log(f"Duplicated {source_path} -> {target_path}")
    return duplicated


aura_system = build_aura_system()


# ============================================================
# MASTER MATERIAL (M_CatAura) — unlit translucent, tintable
# ============================================================

def build_master_material():
    path = f"{MAT_DIR}/{MASTER_MAT_NAME}"

    if asset_exists(path):
        if not OVERWRITE_EXISTING:
            log(f"{MASTER_MAT_NAME} already exists — skipping.")
            return unreal.load_asset(path)
        editor_lib.delete_asset(path)
        warn(f"OVERWRITE_EXISTING=True — deleted existing {path}.")

    factory = unreal.MaterialFactoryNew()
    material = asset_tools.create_asset(MASTER_MAT_NAME, MAT_DIR, unreal.Material, factory)

    if not material:
        fail(f"Failed to create {MASTER_MAT_NAME}.")

    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_SURFACE)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("two_sided", True)

    try:
        tint_param = mat_lib.create_material_expression(
            material, unreal.MaterialExpressionVectorParameter, -400, -100
        )
        tint_param.set_editor_property("parameter_name", "AuraTint")
        tint_param.set_editor_property("default_value", AURA_TINT)
        mat_lib.connect_material_property(
            tint_param, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
        )

        opacity_param = mat_lib.create_material_expression(
            material, unreal.MaterialExpressionScalarParameter, -400, 100
        )
        opacity_param.set_editor_property("parameter_name", "AuraOpacity")
        opacity_param.set_editor_property("default_value", AURA_OPACITY_DEFAULT)
        mat_lib.connect_material_property(
            opacity_param, "", unreal.MaterialProperty.MP_OPACITY
        )

        mat_lib.recompile_material(material)
        log("Wired AuraTint (Emissive Color) and AuraOpacity (Opacity) parameters.")
    except Exception as exc:
        warn(
            f"Could not build the material graph programmatically: {exc} — "
            f"open {path} and wire an Emissive Color (Vector) parameter named "
            f"'AuraTint' and an Opacity (Scalar) parameter named 'AuraOpacity' "
            f"by hand, then Apply/Save."
        )

    save_asset(material)
    log(f"Created {MASTER_MAT_NAME}: {path}")
    return material


master_material = build_master_material()


# ============================================================
# MATERIAL INSTANCE (MI_CatAura_Pink)
# ============================================================

def build_material_instance():
    path = f"{MAT_DIR}/{INSTANCE_NAME}"

    if asset_exists(path):
        if not OVERWRITE_EXISTING:
            log(f"{INSTANCE_NAME} already exists — skipping.")
            return unreal.load_asset(path)
        editor_lib.delete_asset(path)
        warn(f"OVERWRITE_EXISTING=True — deleted existing {path}.")

    if master_material is None:
        warn(f"No master material — cannot create {INSTANCE_NAME}.")
        return None

    factory = unreal.MaterialInstanceConstantFactoryNew()

    instance = asset_tools.create_asset(INSTANCE_NAME, MAT_DIR, unreal.MaterialInstanceConstant, factory)
    if not instance:
        warn(f"Failed to create {INSTANCE_NAME}.")
        return None

    try:
        instance.set_editor_property("parent", master_material)
    except Exception as exc:
        warn(
            f"Could not set parent on {INSTANCE_NAME}: {exc} — open it and set "
            f"Parent = {MASTER_MAT_NAME} manually."
        )

    try:
        mat_lib.set_material_instance_vector_parameter_value(instance, "AuraTint", AURA_TINT)
        mat_lib.set_material_instance_scalar_parameter_value(instance, "AuraOpacity", AURA_OPACITY_DEFAULT)
    except Exception as exc:
        warn(f"Could not preset {INSTANCE_NAME} parameters: {exc} — set AuraTint/AuraOpacity by hand.")

    save_asset(instance)
    log(f"Created {INSTANCE_NAME}: {path}")
    return instance


material_instance = build_material_instance()


# ============================================================
# FINAL VALIDATION
# ============================================================

created_assets = [
    f"{NS_TARGET_DIR}/{NS_TARGET_NAME}",
    f"{MAT_DIR}/{MASTER_MAT_NAME}",
    f"{MAT_DIR}/{INSTANCE_NAME}",
]

log("========================================")
log("CAT AURA SAFE BUILD COMPLETE")
log("========================================")

for path in created_assets:
    if asset_exists(path):
        log(f"OK   {path}")
    else:
        warn(f"MISS {path} — check warnings above and finish manually.")

log("")
log("Remaining manual step regardless of outcome:")
log(f"- Open {NS_TARGET_DIR}/{NS_TARGET_NAME}, select its Sprite/Ribbon Renderer,")
log(f"  and assign {MAT_DIR}/{INSTANCE_NAME} as its Material.")
log("- Then run CatSkill_AutoWire.py to link DA_CatPawSkill.AuraSystem.")
log("SAFE SCRIPT FINISHED.")
