import unreal

# ============================================================
# CAT SKILL — SAFE AUTO-WIRE (Phase 1-2)
# Unreal Engine 5.x
#
# RUN ORDER (important):
#   1. Compile the C++ classes first (CatSkillData / CatSkillComponent /
#      CatPlayerCharacter must already exist as compiled classes in this
#      project — this script cannot create C++ classes, only assets).
#   2. Run CatAura_SafeBuild.py first if NS_CatAura does not exist yet.
#   3. Run this script.
#
# WHAT THIS SCRIPT DOES:
# - Creates IMC_Default / IA_CatSkill if missing (skips if they exist).
# - Maps IA_CatSkill to the E key inside IMC_Default.
# - Creates DA_CatPawSkill (a UCatSkillData instance) if missing, fills
#   in Cooldown / SkillDisplayName / AuraSystem / AuraAttachSocketName.
# - Creates BP_PlayerCatSkill (parent = CatPlayerCharacter) if missing,
#   and sets its class-default references (DefaultMappingContext,
#   CatSkillAction) plus the inherited CatSkillComponent's SkillData.
#
# WHAT THIS SCRIPT DOES NOT DO:
# - Does not touch NS_CatAura's internals (leave that to CatAura_SafeBuild.py).
# - Does not delete or overwrite existing assets unless OVERWRITE_EXISTING=True.
# - Does not open any editor windows automatically.
# ============================================================


# ============================================================
# CONFIG — adjust paths to match your project
# ============================================================

INPUT_DIR = "/Game/Input"
DATA_DIR = "/Game/Data/Skills"
CHAR_DIR = "/Game/Blueprints/Characters"

IMC_NAME = "IMC_Default"
IA_NAME = "IA_CatSkill"
SKILL_DATA_NAME = "DA_CatPawSkill"
PLAYER_BP_NAME = "BP_PlayerCatSkill"

# Path to the Niagara System built by CatAura_SafeBuild.py
NS_CAT_AURA_PATH = "/Game/VFX/CatSkill/Systems/NS_CatAura"

# Name of the C++ classes as they appear to Python. If your module name
# is not "MyProject" the class still exposes under its own class name
# (the MYPROJECT_API macro does not affect this) — only the plugin/module
# needs to be compiled and loaded.
SKILL_DATA_CLASS_NAME = "CatSkillData"
PLAYER_CHARACTER_CLASS_NAME = "CatPlayerCharacter"

AURA_ATTACH_SOCKET = unreal.Name("")  # NAME_None -> attach to root
SKILL_COOLDOWN = 3.0
SKILL_DISPLAY_NAME = "Cat Paw Stun"

# Keep False while testing — avoids clobbering hand-tuned assets.
OVERWRITE_EXISTING = False


# ============================================================
# UTILITIES (same pattern as CatAura_SafeBuild.py)
# ============================================================

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_lib = unreal.EditorAssetLibrary


def log(message):
    unreal.log(f"[CatSkillWire] {message}")


def warn(message):
    unreal.log_warning(f"[CatSkillWire] {message}")


def fail(message):
    unreal.log_error(f"[CatSkillWire] {message}")
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


def find_class_by_name(class_name):
    """
    Resolve a compiled C++ or Blueprint class by its bare name.
    Returns None (with a warning) instead of raising, so the script can
    keep going and report every missing piece at the end.
    """
    found = unreal.EditorAssetLibrary.list_assets("/Script", recursive=True)
    # list_assets on /Script does not reliably enumerate native classes in
    # all engine versions — fall back to direct attribute lookup on the
    # unreal module, which is the normal way native UCLASS/UDataAsset
    # subclasses are exposed to Python once the module is compiled+loaded.
    cls = getattr(unreal, class_name, None)
    if cls is None:
        warn(
            f"Class '{class_name}' not found on the unreal module. "
            f"Make sure the C++ module is compiled and loaded before "
            f"running this script."
        )
    return cls


# ============================================================
# FOLDERS
# ============================================================

for folder in [INPUT_DIR, DATA_DIR, CHAR_DIR]:
    ensure_directory(folder)


# ============================================================
# INPUT ACTION + MAPPING CONTEXT
# ============================================================

def create_input_action():
    path = f"{INPUT_DIR}/{IA_NAME}"

    if asset_exists(path):
        log(f"{IA_NAME} already exists.")
        return unreal.load_asset(path)

    factory_cls = getattr(unreal, "InputActionFactory", None)
    if factory_cls is None:
        warn(
            "unreal.InputActionFactory not available in this engine build — "
            f"create {IA_NAME} manually (Input Action asset, Value Type = Digital/bool)."
        )
        return None

    factory = factory_cls()
    ia = asset_tools.create_asset(IA_NAME, INPUT_DIR, unreal.InputAction, factory)

    if not ia:
        warn(f"Failed to create {IA_NAME}.")
        return None

    save_asset(ia)
    log(f"Created {IA_NAME}: {path}")
    return ia


def create_mapping_context(input_action):
    path = f"{INPUT_DIR}/{IMC_NAME}"

    if asset_exists(path):
        log(f"{IMC_NAME} already exists.")
        return unreal.load_asset(path)

    factory_cls = getattr(unreal, "InputMappingContextFactory", None)
    if factory_cls is None:
        warn(
            "unreal.InputMappingContextFactory not available in this engine build — "
            f"create {IMC_NAME} manually and map {IA_NAME} to the E key."
        )
        return None

    factory = factory_cls()
    imc = asset_tools.create_asset(IMC_NAME, INPUT_DIR, unreal.InputMappingContext, factory)

    if not imc:
        warn(f"Failed to create {IMC_NAME}.")
        return None

    if input_action is not None:
        try:
            imc.map_key(input_action, unreal.Key("E"))
            log(f"Mapped {IA_NAME} -> E inside {IMC_NAME}.")
        except Exception as exc:
            warn(f"Could not auto-map E key: {exc} — map it manually in {IMC_NAME}.")

    save_asset(imc)
    log(f"Created {IMC_NAME}: {path}")
    return imc


cat_skill_action = create_input_action()
default_mapping_context = create_mapping_context(cat_skill_action)


# ============================================================
# SKILL DATA ASSET (UCatSkillData instance)
# ============================================================

def create_skill_data():
    path = f"{DATA_DIR}/{SKILL_DATA_NAME}"

    if asset_exists(path):
        log(f"{SKILL_DATA_NAME} already exists.")
        return unreal.load_asset(path)

    data_class = find_class_by_name(SKILL_DATA_CLASS_NAME)
    if data_class is None:
        return None

    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", data_class)

    data_asset = asset_tools.create_asset(SKILL_DATA_NAME, DATA_DIR, data_class, factory)

    if not data_asset:
        warn(f"Failed to create {SKILL_DATA_NAME}.")
        return None

    data_asset.set_editor_property("skill_display_name", SKILL_DISPLAY_NAME)
    data_asset.set_editor_property("cooldown", SKILL_COOLDOWN)
    data_asset.set_editor_property("aura_attach_socket_name", AURA_ATTACH_SOCKET)

    if asset_exists(NS_CAT_AURA_PATH):
        aura_system = unreal.load_asset(NS_CAT_AURA_PATH)
        data_asset.set_editor_property("aura_system", aura_system)
        log(f"Linked AuraSystem -> {NS_CAT_AURA_PATH}")
    else:
        warn(
            f"{NS_CAT_AURA_PATH} not found — run CatAura_SafeBuild.py first, "
            f"then re-run this script or assign AuraSystem manually."
        )

    save_asset(data_asset)
    log(f"Created {SKILL_DATA_NAME}: {path}")
    return data_asset


skill_data = create_skill_data()


# ============================================================
# PLAYER BLUEPRINT (parent = CatPlayerCharacter)
# ============================================================

def create_player_blueprint():
    path = f"{CHAR_DIR}/{PLAYER_BP_NAME}"

    parent_class = find_class_by_name(PLAYER_CHARACTER_CLASS_NAME)
    if parent_class is None:
        return None

    if asset_exists(path):
        log(f"{PLAYER_BP_NAME} already exists.")
        return unreal.load_asset(path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    bp = asset_tools.create_asset(PLAYER_BP_NAME, CHAR_DIR, unreal.Blueprint, factory)

    if not bp:
        warn(f"Failed to create {PLAYER_BP_NAME}.")
        return None

    save_asset(bp)
    log(f"Created {PLAYER_BP_NAME}: {path}")
    return bp


def wire_player_blueprint(bp):
    if bp is None:
        return

    try:
        generated_class = bp.generated_class()
        cdo = unreal.get_default_object(generated_class)
    except Exception as exc:
        warn(f"Could not access {PLAYER_BP_NAME} class defaults: {exc}")
        return

    if default_mapping_context is not None:
        try:
            cdo.set_editor_property("default_mapping_context", default_mapping_context)
        except Exception as exc:
            warn(f"Could not set DefaultMappingContext: {exc}")

    if cat_skill_action is not None:
        try:
            cdo.set_editor_property("cat_skill_action", cat_skill_action)
        except Exception as exc:
            warn(f"Could not set CatSkillAction: {exc}")

    if skill_data is not None:
        try:
            skill_component = cdo.get_editor_property("cat_skill_component")
            if skill_component is not None:
                skill_component.set_editor_property("skill_data", skill_data)
                log("Linked CatSkillComponent.SkillData -> DA_CatPawSkill")
        except Exception as exc:
            warn(f"Could not set CatSkillComponent.SkillData: {exc}")

    save_asset(bp)


player_bp = create_player_blueprint()
wire_player_blueprint(player_bp)


# ============================================================
# FINAL VALIDATION
# ============================================================

created_assets = [
    f"{INPUT_DIR}/{IA_NAME}",
    f"{INPUT_DIR}/{IMC_NAME}",
    f"{DATA_DIR}/{SKILL_DATA_NAME}",
    f"{CHAR_DIR}/{PLAYER_BP_NAME}",
]

log("========================================")
log("CAT SKILL AUTO-WIRE COMPLETE")
log("========================================")

for path in created_assets:
    if asset_exists(path):
        log(f"OK   {path}")
    else:
        warn(f"MISS {path} — check warnings above and finish wiring manually.")

log("")
log("Remaining manual step regardless of outcome:")
log("- Drag BP_PlayerCatSkill into your test level and set it as the default pawn,")
log("  or set it as the GameMode's Default Pawn Class.")
log("SAFE SCRIPT FINISHED.")
