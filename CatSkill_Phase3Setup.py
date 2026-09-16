import unreal

# ============================================================
# CAT SKILL — PHASE 3-8 SAFE SETUP
# Unreal Engine 5.x
#
# RUN ORDER (important):
#   1. Compile the Phase 3-10 C++ classes first (CatPawProjectile,
#      EffectReceiverComponent, StatusComponent, EconomyComponent,
#      AttachmentComponent, CarryItemData, InteractionComponent must
#      already be compiled classes in this project).
#   2. Run this script.
#   3. Open DA_CatPawSkill and eyeball the Effects it wrote (durations,
#      magnitudes) — tune to taste, this script just seeds defaults.
#   4. Add a placeholder StaticMesh/material to BP_PawProjectile and
#      BP_CatTreat by hand — this script cannot author visuals.
#
# WHAT THIS SCRIPT DOES:
# - Creates BP_PawProjectile (parent = CatPawProjectile) if missing.
# - Creates BP_CatTreat (parent = Actor) if missing.
# - Creates DA_CatTreat (a UCarryItemData instance) if missing, pointing
#   ActorClass at BP_CatTreat.
# - Creates BP_EnemyTest (parent = Character) if missing, adds
#   EffectReceiverComponent / StatusComponent / EconomyComponent /
#   AttachmentComponent to it, and sets AutoPossessAI so
#   InteractionComponent's AIController lookup works without manual setup.
# - Updates the existing DA_CatPawSkill with Range/ProjectileClass/Effects
#   (Stun 2.5s, GoldCost 10, CarryItem -> DA_CatTreat, MoveToActor).
#
# WHAT THIS SCRIPT DOES NOT DO:
# - Does not add a mesh to BP_PawProjectile or BP_CatTreat — placeholder
#   visuals are your call, not this script's.
# - Does not delete or overwrite existing assets unless OVERWRITE_EXISTING=True.
# - Does not touch NS_CatAura or the Input assets — that's CatAura_SafeBuild.py
#   and CatSkill_AutoWire.py's job.
# ============================================================


# ============================================================
# CONFIG — adjust paths to match your project
# ============================================================

ROOT_DIR = "/Game/CatSkill"

PROJECTILE_BP_NAME = "BP_PawProjectile"
ENEMY_BP_NAME = "BP_EnemyTest"
TREAT_DATA_NAME = "DA_CatTreat"
TREAT_ACTOR_NAME = "BP_CatTreat"
SKILL_DATA_PATH = f"{ROOT_DIR}/DA_CatPawSkill"

PROJECTILE_CLASS_NAME = "CatPawProjectile"
CARRY_ITEM_DATA_CLASS_NAME = "CarryItemData"

SKILL_RANGE = 800.0
STUN_DURATION = 2.5
GOLD_COST = 10.0

# Keep False while testing — avoids clobbering hand-tuned assets.
OVERWRITE_EXISTING = False


# ============================================================
# UTILITIES (same pattern as CatSkill_AutoWire.py / CatAura_SafeBuild.py)
# ============================================================

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_lib = unreal.EditorAssetLibrary


def log(message):
    unreal.log(f"[CatSkillPhase3] {message}")


def warn(message):
    unreal.log_warning(f"[CatSkillPhase3] {message}")


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
    cls = getattr(unreal, class_name, None)
    if cls is None:
        warn(
            f"Class '{class_name}' not found on the unreal module. "
            f"Make sure the C++ module is compiled and loaded before running this script."
        )
    return cls


ensure_directory(ROOT_DIR)


# ============================================================
# BP_PawProjectile
# ============================================================

def create_projectile_blueprint():
    path = f"{ROOT_DIR}/{PROJECTILE_BP_NAME}"

    if asset_exists(path):
        log(f"{PROJECTILE_BP_NAME} already exists.")
        return unreal.load_asset(path)

    parent_class = find_class_by_name(PROJECTILE_CLASS_NAME)
    if parent_class is None:
        return None

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    bp = asset_tools.create_asset(PROJECTILE_BP_NAME, ROOT_DIR, unreal.Blueprint, factory)
    if not bp:
        warn(f"Failed to create {PROJECTILE_BP_NAME}.")
        return None

    save_asset(bp)
    log(f"Created {PROJECTILE_BP_NAME}: {path} — add a placeholder mesh to it by hand.")
    return bp


projectile_bp = create_projectile_blueprint()


# ============================================================
# BP_CatTreat
# ============================================================

def create_treat_actor_blueprint():
    path = f"{ROOT_DIR}/{TREAT_ACTOR_NAME}"

    if asset_exists(path):
        log(f"{TREAT_ACTOR_NAME} already exists.")
        return unreal.load_asset(path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.Actor)

    bp = asset_tools.create_asset(TREAT_ACTOR_NAME, ROOT_DIR, unreal.Blueprint, factory)
    if not bp:
        warn(f"Failed to create {TREAT_ACTOR_NAME}.")
        return None

    save_asset(bp)
    log(f"Created {TREAT_ACTOR_NAME}: {path} — add a StaticMeshComponent + mesh to it by hand.")
    return bp


treat_actor_bp = create_treat_actor_blueprint()


# ============================================================
# DA_CatTreat (UCarryItemData instance)
# ============================================================

def create_treat_data():
    path = f"{ROOT_DIR}/{TREAT_DATA_NAME}"

    if asset_exists(path):
        log(f"{TREAT_DATA_NAME} already exists.")
        return unreal.load_asset(path)

    data_class = find_class_by_name(CARRY_ITEM_DATA_CLASS_NAME)
    if data_class is None:
        return None

    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", data_class)

    data_asset = asset_tools.create_asset(TREAT_DATA_NAME, ROOT_DIR, data_class, factory)
    if not data_asset:
        warn(f"Failed to create {TREAT_DATA_NAME}.")
        return None

    data_asset.set_editor_property("item_id", unreal.Name("CatTreat"))

    if treat_actor_bp is not None:
        try:
            data_asset.set_editor_property("actor_class", treat_actor_bp.generated_class())
        except Exception as exc:
            warn(f"Could not set {TREAT_DATA_NAME}.ActorClass: {exc} — set it manually to {TREAT_ACTOR_NAME}.")
    else:
        warn(f"{TREAT_ACTOR_NAME} was not created — set {TREAT_DATA_NAME}.ActorClass manually.")

    save_asset(data_asset)
    log(f"Created {TREAT_DATA_NAME}: {path}")
    return data_asset


treat_data = create_treat_data()


# ============================================================
# BP_EnemyTest (parent = Character, + gameplay components)
# ============================================================

def add_component_to_blueprint(bp, component_class, component_name):
    """
    Adds an ActorComponent to a Blueprint's SimpleConstructionScript via the
    SubobjectDataSubsystem. Returns True on success — warns (does not raise)
    on failure so one bad component doesn't stop the rest of the script.
    """
    try:
        subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        root_handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
        if not root_handles:
            warn(f"{bp.get_name()}: no root subobject handle — cannot add {component_name}.")
            return False

        params = unreal.AddNewSubobjectParams(
            parent_handle=root_handles[0],
            new_class=component_class,
            blueprint_context=bp,
        )
        new_handle, fail_reason = subsystem.add_new_subobject(params)
        if fail_reason and str(fail_reason):
            warn(f"{bp.get_name()}: add_new_subobject({component_name}) reported: {fail_reason}")

        subsystem.rename_subobject(handle=new_handle, new_name=unreal.Text(component_name))
        return True
    except Exception as exc:
        warn(
            f"Could not add {component_name} to {bp.get_name()} via script: {exc} — "
            f"open the Blueprint and add it manually (Add Component -> {component_name})."
        )
        return False


def create_enemy_test_blueprint():
    path = f"{ROOT_DIR}/{ENEMY_BP_NAME}"

    if asset_exists(path):
        log(f"{ENEMY_BP_NAME} already exists.")
        return unreal.load_asset(path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.Character)

    bp = asset_tools.create_asset(ENEMY_BP_NAME, ROOT_DIR, unreal.Blueprint, factory)
    if not bp:
        warn(f"Failed to create {ENEMY_BP_NAME}.")
        return None

    for component_class_name, component_name in [
        ("EffectReceiverComponent", "EffectReceiverComponent"),
        ("StatusComponent", "StatusComponent"),
        ("EconomyComponent", "EconomyComponent"),
        ("AttachmentComponent", "AttachmentComponent"),
    ]:
        component_class = find_class_by_name(component_class_name)
        if component_class is not None:
            add_component_to_blueprint(bp, component_class, component_name)

    try:
        generated_class = bp.generated_class()
        cdo = unreal.get_default_object(generated_class)
        cdo.set_editor_property("auto_possess_ai", unreal.AutoPossessAI.PLACED_IN_WORLD_OR_SPAWNED)
        cdo.set_editor_property("ai_controller_class", unreal.AIController)
        log(f"{ENEMY_BP_NAME}: set AutoPossessAI + AIControllerClass so InteractionComponent can MoveToActor.")
    except Exception as exc:
        warn(
            f"Could not set AutoPossessAI/AIControllerClass on {ENEMY_BP_NAME}: {exc} — "
            f"set them manually in Class Defaults (Pawn category) for Phase 9 to work."
        )

    save_asset(bp)
    log(f"Created {ENEMY_BP_NAME}: {path}")
    return bp


enemy_bp = create_enemy_test_blueprint()


# ============================================================
# UPDATE DA_CatPawSkill — Range / ProjectileClass / Effects
# ============================================================

def build_effect_spec(effect_type_name, **fields):
    effect_type_enum = getattr(unreal, "GameplayEffectType", None)
    if effect_type_enum is None:
        warn("unreal.GameplayEffectType not found — is GameplayEffectTypes.h compiled in?")
        return None

    effect_type = getattr(effect_type_enum, effect_type_name, None)
    if effect_type is None:
        warn(f"unreal.GameplayEffectType has no member '{effect_type_name}'.")
        return None

    spec = unreal.GameplayEffectSpec()
    spec.set_editor_property("type", effect_type)
    for field_name, value in fields.items():
        try:
            spec.set_editor_property(field_name, value)
        except Exception as exc:
            warn(f"Could not set GameplayEffectSpec.{field_name}: {exc}")
    return spec


def update_skill_data():
    if not asset_exists(SKILL_DATA_PATH):
        warn(f"{SKILL_DATA_PATH} not found — run CatSkill_AutoWire.py first, then re-run this script.")
        return

    skill_data = unreal.load_asset(SKILL_DATA_PATH)
    if not skill_data:
        warn(f"Failed to load {SKILL_DATA_PATH}.")
        return

    try:
        skill_data.set_editor_property("range", SKILL_RANGE)
    except Exception as exc:
        warn(f"Could not set DA_CatPawSkill.Range: {exc}")

    if projectile_bp is not None:
        try:
            skill_data.set_editor_property("projectile_class", projectile_bp.generated_class())
        except Exception as exc:
            warn(f"Could not set DA_CatPawSkill.ProjectileClass: {exc}")
    else:
        warn(f"{PROJECTILE_BP_NAME} was not created — set DA_CatPawSkill.ProjectileClass manually.")

    effects = []
    stun = build_effect_spec("STUN", duration=STUN_DURATION)
    if stun is not None:
        effects.append(stun)

    gold = build_effect_spec("GOLD_COST", magnitude=GOLD_COST)
    if gold is not None:
        effects.append(gold)

    if treat_data is not None:
        carry = build_effect_spec("CARRY_ITEM", asset=treat_data)
        if carry is not None:
            effects.append(carry)
    else:
        warn(f"{TREAT_DATA_NAME} was not created — skipping the CarryItem effect entry.")

    move = build_effect_spec("MOVE_TO_ACTOR")
    if move is not None:
        effects.append(move)

    if effects:
        try:
            skill_data.set_editor_property("effects", effects)
            log(f"DA_CatPawSkill.Effects set to {len(effects)} entries "
                f"(Stun {STUN_DURATION}s, GoldCost {GOLD_COST}, CarryItem, MoveToActor).")
        except Exception as exc:
            warn(f"Could not set DA_CatPawSkill.Effects: {exc}")

    save_asset(skill_data)
    log(f"Updated {SKILL_DATA_PATH}.")


update_skill_data()


# ============================================================
# FINAL VALIDATION
# ============================================================

created_assets = [
    f"{ROOT_DIR}/{PROJECTILE_BP_NAME}",
    f"{ROOT_DIR}/{TREAT_ACTOR_NAME}",
    f"{ROOT_DIR}/{TREAT_DATA_NAME}",
    f"{ROOT_DIR}/{ENEMY_BP_NAME}",
]

log("========================================")
log("CAT SKILL PHASE 3-8 SETUP COMPLETE")
log("========================================")

for path in created_assets:
    if asset_exists(path):
        log(f"OK   {path}")
    else:
        warn(f"MISS {path} — check warnings above and finish manually.")

log("")
log("Remaining manual steps regardless of outcome:")
log(f"- Add a placeholder mesh to {PROJECTILE_BP_NAME} and {TREAT_ACTOR_NAME}.")
log(f"- Open {ENEMY_BP_NAME} and confirm the 4 components were added — add any that are missing.")
log(f"- Drag {ENEMY_BP_NAME} into your test level within skill range of the player.")
log("SAFE SCRIPT FINISHED.")
