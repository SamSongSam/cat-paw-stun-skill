import unreal

# ============================================================
# CAT SKILL — VFX TEMPLATE SCAFFOLD (sections 22-27)
# Unreal Engine 5.x
#
# Creates EMPTY placeholder assets for every Niagara System, Material, and
# Material Function CatSkill_test.md names in its Content folder structure
# (section 34) that isn't built yet. These are starting points only — no
# emitter graphs, no shader math. Open each one in its editor and build the
# actual effect per the doc (sections 23, 25-27).
#
# WHAT THIS SCRIPT DOES:
# - Creates blank Niagara Systems: NS_CatPawProjectile, NS_CatPawImpact,
#   NS_EnemyStun (NS_CatAura already exists from CatAura_SafeBuild.py).
# - Creates blank unlit/translucent Materials: M_CatPaw_SDF, M_CatShell_SDF,
#   M_CatTrail.
# - Creates blank Material Functions: MF_SDFCircle, MF_SDFEllipse,
#   MF_SmoothUnion, MF_PawShape, MF_ShellSpiral.
#
# WHAT THIS SCRIPT DOES NOT DO:
# - Does not build any emitter/module/node graph — Niagara stack editing and
#   material graph editing are not reliably scriptable (see
#   CatAura_SafeBuild.py's own notes on this, learned the hard way this
#   session).
# - Does not delete or overwrite existing assets unless OVERWRITE_EXISTING=True.
# ============================================================


# ============================================================
# CONFIG
# ============================================================

SYSTEMS_DIR = "/Game/CatSkill"
MATERIALS_DIR = "/Game/CatSkill/Materials"
FUNCTIONS_DIR = "/Game/CatSkill/Materials/Functions"

NIAGARA_SYSTEMS = ["NS_CatPawProjectile", "NS_CatPawImpact", "NS_EnemyStun"]
MATERIALS = ["M_CatPaw_SDF", "M_CatShell_SDF", "M_CatTrail"]
MATERIAL_FUNCTIONS = ["MF_SDFCircle", "MF_SDFEllipse", "MF_SmoothUnion", "MF_PawShape", "MF_ShellSpiral"]

OVERWRITE_EXISTING = False


# ============================================================
# UTILITIES (same pattern as the other CatSkill_*.py scripts)
# ============================================================

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_lib = unreal.EditorAssetLibrary


def log(message):
    unreal.log(f"[CatSkillVFXTemplates] {message}")


def warn(message):
    unreal.log_warning(f"[CatSkillVFXTemplates] {message}")


def ensure_directory(path):
    if not editor_lib.does_directory_exist(path):
        editor_lib.make_directory(path)
        log(f"Created folder: {path}")


def asset_exists(path):
    return editor_lib.does_asset_exist(path)


def save_asset(asset):
    if asset:
        editor_lib.save_loaded_asset(asset, only_if_is_dirty=False)


for folder in (SYSTEMS_DIR, MATERIALS_DIR, FUNCTIONS_DIR):
    ensure_directory(folder)


# ============================================================
# NIAGARA SYSTEMS — blank templates
# ============================================================

def create_blank_niagara_system(name):
    path = f"{SYSTEMS_DIR}/{name}"

    if asset_exists(path):
        if not OVERWRITE_EXISTING:
            log(f"{name} already exists — skipping.")
            return
        editor_lib.delete_asset(path)
        warn(f"OVERWRITE_EXISTING=True — deleted existing {path}.")

    factory = unreal.NiagaraSystemFactoryNew()
    system = asset_tools.create_asset(name, SYSTEMS_DIR, unreal.NiagaraSystem, factory)

    if not system:
        warn(f"Failed to create {name}.")
        return

    save_asset(system)
    log(f"Created {name}: {path} — empty, build the emitter graph in the Niagara editor.")


for ns_name in NIAGARA_SYSTEMS:
    create_blank_niagara_system(ns_name)


# ============================================================
# MATERIALS — blank unlit/translucent templates
# ============================================================

def create_blank_material(name):
    path = f"{MATERIALS_DIR}/{name}"

    if asset_exists(path):
        if not OVERWRITE_EXISTING:
            log(f"{name} already exists — skipping.")
            return
        editor_lib.delete_asset(path)
        warn(f"OVERWRITE_EXISTING=True — deleted existing {path}.")

    factory = unreal.MaterialFactoryNew()
    material = asset_tools.create_asset(name, MATERIALS_DIR, unreal.Material, factory)

    if not material:
        warn(f"Failed to create {name}.")
        return

    try:
        material.set_editor_property("material_domain", unreal.MaterialDomain.MD_SURFACE)
        material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        material.set_editor_property("two_sided", True)
    except Exception as exc:
        warn(f"Could not preset {name}'s domain/blend/shading: {exc}")

    save_asset(material)
    log(f"Created {name}: {path} — empty, build the SDF/shape math in the Material editor.")


for mat_name in MATERIALS:
    create_blank_material(mat_name)


# ============================================================
# MATERIAL FUNCTIONS — blank templates
# ============================================================

def create_blank_material_function(name):
    path = f"{FUNCTIONS_DIR}/{name}"

    if asset_exists(path):
        if not OVERWRITE_EXISTING:
            log(f"{name} already exists — skipping.")
            return
        editor_lib.delete_asset(path)
        warn(f"OVERWRITE_EXISTING=True — deleted existing {path}.")

    factory = unreal.MaterialFunctionFactoryNew()
    material_function = asset_tools.create_asset(name, FUNCTIONS_DIR, unreal.MaterialFunction, factory)

    if not material_function:
        warn(f"Failed to create {name}.")
        return

    save_asset(material_function)
    log(f"Created {name}: {path} — empty, build the SDF math node graph in the Material Function editor.")


for mf_name in MATERIAL_FUNCTIONS:
    create_blank_material_function(mf_name)


# ============================================================
# FINAL VALIDATION
# ============================================================

created_assets = (
    [f"{SYSTEMS_DIR}/{n}" for n in NIAGARA_SYSTEMS]
    + [f"{MATERIALS_DIR}/{n}" for n in MATERIALS]
    + [f"{FUNCTIONS_DIR}/{n}" for n in MATERIAL_FUNCTIONS]
)

log("========================================")
log("VFX TEMPLATE SCAFFOLD COMPLETE")
log("========================================")

for path in created_assets:
    if asset_exists(path):
        log(f"OK   {path}")
    else:
        warn(f"MISS {path} — check warnings above.")

log("")
log("Reminder: these are EMPTY placeholders. Build order per section 12-14/25-27:")
log("  1. MF_SDFCircle / MF_SDFEllipse / MF_SmoothUnion -> paw shape math")
log("  2. MF_PawShape combines them (Main Pad + 4 toes)")
log("  3. M_CatPaw_SDF uses MF_PawShape for its Emissive mask")
log("  4. NS_CatPawProjectile / NS_CatPawImpact / NS_EnemyStun -> build emitters, assign materials")
log("SAFE SCRIPT FINISHED.")
