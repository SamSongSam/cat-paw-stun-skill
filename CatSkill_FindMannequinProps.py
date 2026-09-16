import unreal

# ============================================================
# DIAGNOSTIC ONLY — find any actor in the current level whose name or
# mesh looks like a leftover default mannequin prop (not our BP_PlayerCatSkill
# or BP_EnemyTest), and report its location relative to the PlayerStart.
# ============================================================

def log(message):
    unreal.log(f"[CatSkillMannequinFind] {message}")


subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = subsystem.get_all_level_actors()

player_start_loc = None
for actor in all_actors:
    if isinstance(actor, unreal.PlayerStart):
        player_start_loc = actor.get_actor_location()
        log(f"PlayerStart '{actor.get_name()}' at {player_start_loc}")

log(f"Total actors in level: {len(all_actors)}")
log("Scanning for mannequin-looking actors...")

for actor in all_actors:
    name = actor.get_name()
    class_name = actor.get_class().get_name()
    label = actor.get_actor_label()

    is_suspect = "mann" in name.lower() or "mann" in label.lower() or "quinn" in name.lower() or "quinn" in label.lower()

    if is_suspect:
        loc = actor.get_actor_location()
        dist_to_start = "n/a"
        if player_start_loc is not None:
            dist_to_start = f"{(loc - player_start_loc).size():.1f}"
        log(f"SUSPECT: Label='{label}' Name='{name}' Class='{class_name}' "
            f"Location={loc} DistanceToPlayerStart={dist_to_start}")

log("Scanning ALL actors with a SkeletalMeshComponent using a Manny/Quinn mesh...")

for actor in all_actors:
    mesh_comp = None
    try:
        mesh_comp = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    except Exception:
        mesh_comp = None

    if mesh_comp is None:
        continue

    try:
        skm = mesh_comp.get_editor_property("skeletal_mesh_asset")
    except Exception:
        try:
            skm = mesh_comp.get_editor_property("skeletal_mesh")
        except Exception:
            skm = None

    skm_name = skm.get_name() if skm is not None else "None"
    if "manny" in skm_name.lower() or "quinn" in skm_name.lower():
        loc = actor.get_actor_location()
        dist_to_start = "n/a"
        if player_start_loc is not None:
            dist_to_start = f"{(loc - player_start_loc).size():.1f}"
        log(f"MESH MATCH: Label='{actor.get_actor_label()}' Class='{actor.get_class().get_name()}' "
            f"SkeletalMesh='{skm_name}' Location={loc} DistanceToPlayerStart={dist_to_start}")

log("DIAGNOSTIC FINISHED.")
