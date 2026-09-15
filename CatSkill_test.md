ได้ กูสรุป **FULL STRUCTURE ของ Skill นี้แบบรวมทุกอย่างที่คุยกันมา** ให้เป็นโครงที่เอาไปทำจริงใน UE ได้เลย ทั้ง Gameplay, C++, BP, Niagara, Material, Enemy response, Item handoff, AI, Animation, Data, Debug, Portfolio

# CAT PAW STUN SKILL — FULL STRUCTURE

## 1. Skill Concept

สกิลหลัก:

```text
Press E
↓
Activate Cat Aura
↓
Spawn Procedural Cat Paw Projectiles
↓
Paw Dash ไปหา Enemy
↓
Hit Enemy
↓
Enemy Stunned
↓
Enemy เสีย Gold
↓
Enemy/NPC ถือ "แมวเลีย"
↓
เดินมาหา Player
↓
ส่งแมวเลียให้ Player/Cat
↓
Cat รับของ
↓
Cat Lick / Eat Animation
↓
Skill End
```

แก่นของระบบคือ:

```text
Skill ไม่รู้ Enemy ชนิดไหน
Enemy ไม่รู้ Skill ของ Hero ตัวไหน
Item ไม่รู้ว่าใครถือ
Niagara ไม่ถือ Gameplay Logic
Animation ไม่เป็นเจ้าของ Gameplay State
```

ทุกอย่างคุยผ่าน:

```text
Effect
Component
Event
Data Asset
```

---

# 2. High-Level Architecture

```text
INPUT
│
└── IA_CatSkill
        │
        ▼
BP_PlayerCatSkill
        │
        ▼
UCatSkillComponent
        │
        ├── SkillData
        │
        ├── Target Detection
        │
        ├── Niagara Aura
        │
        └── Projectile Spawn
                │
                ▼
        BP_PawProjectile
                │
                ▼
            HIT TARGET
                │
                ▼
        UEffectReceiverComponent
                │
     ┌──────────┼──────────┬─────────────┐
     ▼          ▼          ▼             ▼
 Status     Economy    Attachment     Interaction
Component   Component   Component      Component
     │          │          │             │
     │          │          │             └── AI Move / Deliver
     │          │          │
     │          │          └── Spawn Cat Treat
     │          │
     │          └── Pay Gold
     │
     └── Apply Stun
```

---

# 3. Player Structure

## `BP_PlayerCatSkill`

ตัวละครทดลองของเรา

```text
BP_PlayerCatSkill
├─ CapsuleComponent
├─ Mesh
├─ CharacterMovement
├─ CatSkillComponent
├─ Camera
└─ SpringArm
```

ตอนนี้เราใช้ mannequin ไปก่อน

ภายหลังเปลี่ยน mesh ได้โดยไม่กระทบ gameplay logic

---

# 4. Input

## Enhanced Input

Assets:

```text
IA_CatSkill
IMC_Default
```

Mapping:

```text
IA_CatSkill
→ E
```

Runtime:

```text
BeginPlay
↓
Enhanced Input Local Player Subsystem
↓
Add Mapping Context
↓
IMC_Default
```

Input Flow:

```text
IA_CatSkill
Started
↓
CatSkillComponent.ActivateCatSkill()
```

ไม่ใช้ Tick เช็ก E

---

# 5. Core C++ Component

## `UCatSkillComponent`

หน้าที่:

```text
รับคำสั่ง Activate Skill
ควบคุม cooldown
spawn aura
หา target
spawn projectile
ส่ง gameplay effect
แจ้ง Blueprint presentation event
```

ไม่ควร:

```text
เขียน logic ของ enemy
เขียน AI ของ enemy
ถือ inventory ของ enemy
ควบคุม animation enemy โดยตรง
```

### Properties

```cpp
SkillData
AuraSystem
ProjectileClass

SkillCooldown
SkillRange
ProjectileSpeed

bIsSkillActive
LastCastTime
```

### Functions

```cpp
ActivateCatSkill()

CanActivateSkill()

FindTarget()

SpawnAura()

SpawnProjectile()

ApplySkillEffects()

ResetSkill()
```

### Blueprint Events

```cpp
OnCatSkillActivated()

OnTargetAcquired()

OnProjectileSpawned()

OnSkillHit()

OnSkillFinished()
```

---

# 6. Skill Data

อย่า hardcode ค่าทั้งหมดใน Component

สร้าง:

```text
DA_CatPawSkill
```

Class:

```text
UCatSkillData
```

มี:

```text
SkillName
Cooldown
Range
ProjectileSpeed

AuraSystem
ProjectileClass

ImpactSystem

Effects[]
```

Effects ของ skill:

```text
Stun
GoldCost
CarryItem
MoveToPlayer
TransferItem
```

ดังนั้นวันหลังสร้างสกิลใหม่:

```text
DA_FireSkill
DA_IceSkill
DA_FishSkill
DA_HeartSkill
```

ไม่ต้องเขียน enemy ใหม่

---

# 7. Gameplay Effect Layer

นี่คือหัวใจ architecture

สร้าง:

```text
FGameplayEffectSpec
```

หรือถ้าอยากทำ Data Asset:

```text
DA_GameplayEffect
```

Effect Types:

```cpp
enum class EGameplayEffectType
{
    Stun,
    Slow,
    Burn,
    Knockback,

    GoldCost,

    CarryItem,
    TransferItem,

    MoveToActor,

    PlayInteraction,

    Custom
};
```

Effect Spec:

```text
Type
Magnitude
Duration
Target
Source
Asset
Tag
```

ตัวอย่าง Skill นี้:

```text
Effect 1
Type = Stun
Duration = 2.5

Effect 2
Type = GoldCost
Magnitude = 10

Effect 3
Type = CarryItem
Asset = DA_CatTreat

Effect 4
Type = MoveToActor
Target = SkillOwner

Effect 5
Type = TransferItem
Target = SkillOwner
```

---

# 8. Enemy Structure

Base Enemy:

```text
AEnemyBase
├─ SkeletalMesh
├─ AIController
├─ EffectReceiverComponent
├─ StatusComponent
├─ EconomyComponent
├─ AttachmentComponent
├─ InteractionComponent
└─ AnimationComponent / AnimBP
```

Enemy ไม่ต้องรู้ว่าโดน:

```text
CatSkill
FireSkill
Character A
Character B
```

มันรู้แค่:

> “มี Effect เข้ามา”

---

# 9. Effect Receiver

## `UEffectReceiverComponent`

ทำหน้าที่เป็น gateway

```text
ReceiveEffect()
↓
ดู EffectType
↓
ส่งต่อไป Component ที่เกี่ยวข้อง
```

ตัวอย่าง:

```text
Stun
→ StatusComponent

GoldCost
→ EconomyComponent

CarryItem
→ AttachmentComponent

MoveToActor
→ InteractionComponent

TransferItem
→ Attachment + Interaction
```

pseudo:

```cpp
void UEffectReceiverComponent::ReceiveEffect(
    const FGameplayEffectSpec& Effect)
{
    switch (Effect.Type)
    {
        case Stun:
            StatusComponent->ApplyStatus(Effect);
            break;

        case GoldCost:
            EconomyComponent->ApplyCost(Effect);
            break;

        case CarryItem:
            AttachmentComponent->EquipItem(Effect);
            break;

        case MoveToActor:
            InteractionComponent->MoveToTarget(Effect);
            break;
    }
}
```

---

# 10. Status Component

## `UStatusComponent`

รองรับ:

```text
Stun
Slow
Burn
Freeze
Silence
Knockdown
```

Skill นี้ใช้:

```text
Stun
Duration = 2.5 sec
```

Flow:

```text
Receive Stun
↓
Stop movement
↓
Disable relevant AI
↓
Play stunned state
↓
Trigger Niagara
↓
Timer
↓
Remove Stun
```

Events:

```text
OnStatusApplied
OnStunStarted
OnStunEnded
```

---

# 11. Economy Component

## `UEconomyComponent`

ไม่ควรผูกกับ skill โดยตรง

Properties:

```text
CurrentGold
MaxGold
```

Functions:

```text
CanAfford()
SpendGold()
AddGold()
```

Skill:

```text
Enemy stunned
↓
Spend 10 Gold
```

ถ้าไม่มี Gold:

สามารถเลือก policy:

```text
Fail interaction
หรือ
Use fallback
หรือ
Allow debt
```

ตอน prototype เอาแค่:

```text
Gold > 0
→ Spend Gold
```

---

# 12. Carry Item System

นี่คือระบบที่คุยกันเรื่อง:

```text
ดาบ
หมวก
อาหาร
แมวเลีย
ของ quest
```

อย่า preload ทุก item ใส่ enemy

สร้าง:

```text
UAttachmentComponent
```

หรือชื่อ:

```text
UEquipmentComponent
```

---

# 13. Attachment Points

Character Skeleton มี semantic sockets:

```text
Hand_R
Hand_L
Head
Mouth
Back
Chest
```

ไม่ hardcode bone name กระจายทั่ว code

สร้าง enum:

```text
EAttachmentSlot
{
    RightHand,
    LeftHand,
    Head,
    Mouth,
    Back
}
```

จากนั้น map:

```text
RightHand → hand_r_socket
LeftHand  → hand_l_socket
Head      → head_socket
```

---

# 14. Carry Item Data

สร้าง:

```text
DA_CatTreat
```

Class:

```text
UCarryItemData
```

Properties:

```text
ItemID

ActorClass

AttachmentSlot

SocketName

LocationOffset
RotationOffset
Scale

PickupAnimation

CarryAnimation

GiveAnimation

NiagaraFX
Sound
```

Skill ไม่ต้องรู้ว่า mesh ของแมวเลียคืออะไร

มันบอกแค่:

```text
CarryItem = DA_CatTreat
```

---

# 15. Cat Treat Actor

```text
BP_CatTreat
```

Components:

```text
SceneRoot
StaticMesh
Optional Niagara
Optional Collision
```

Lifecycle:

```text
Spawn
↓
Attach Enemy Hand
↓
Enemy walks
↓
Detach
↓
Transfer
↓
Attach Cat Mouth / Hand
↓
Eat
↓
Destroy / Pool
```

---

# 16. Interaction Component

## `UInteractionComponent`

ดูแล:

```text
Move To Actor
Face Target
Interact
Give Item
Receive Item
```

Skill Flow:

```text
Enemy stunned
↓
Pay Gold
↓
Spawn Cat Treat
↓
Attach Right Hand
↓
MoveTo(Player)
↓
Reach Player
↓
Face Player
↓
Give Item
↓
Transfer
```

Event:

```text
OnInteractionStarted
OnReachedTarget
OnGiveItem
OnInteractionFinished
```

---

# 17. AI Flow

Enemy AI:

```text
Normal AI
↓
Hit by Skill
↓
Stunned
↓
Temporary Interaction State
↓
Carry Cat Treat
↓
Move To Player
↓
Deliver
↓
Return Normal AI
```

State enum:

```text
Normal
Stunned
CarryingItem
MovingToTarget
DeliveringItem
Recovering
```

ไม่ต้องยัดเข้า skill component

---

# 18. Item Transfer

Flow:

```text
Enemy AttachmentComponent
owns ItemActor
↓
OnGiveItem
↓
DetachFromActor
↓
Player AttachmentComponent
AttachItem()
```

หรือ:

```text
Enemy
↓
TransferOwnership()
↓
Player
```

Item ตัวเดิมย้าย holder

ไม่จำเป็นต้อง destroy แล้ว spawn ใหม่

---

# 19. Player Receive Item

Player มี:

```text
AttachmentComponent
```

เมื่อรับ Cat Treat:

```text
Attach → Mouth / Hand
↓
OnItemReceived
↓
Play Cat Lick
```

---

# 20. Cat Lick Sequence

หลังรับแมวเลีย:

```text
Receive Treat
↓
Anim Montage
↓
Anim Notify
↓
Lick Event
↓
Niagara sparkle
↓
Sound
↓
Consume Item
↓
Detach
↓
Destroy / Pool
```

Anim Notify สำคัญ เพราะ:

```text
Gameplay
ไม่ต้องเดา timing animation
```

เช่น:

```text
Notify_TreatTouch
Notify_Lick
Notify_TreatConsumed
```

---

# 21. Animation Architecture

```text
Animation Blueprint
├─ Locomotion
├─ Idle
├─ Stunned
├─ Carry
└─ Interaction
```

Animation Montage:

```text
AM_Enemy_GiveTreat
AM_Cat_ReceiveTreat
AM_Cat_LickTreat
```

Control Rig:

ใช้ทีหลังถ้าต้อง:

```text
ปรับมือจับ item
IK
look-at
procedural pose
```

ไม่ใช้ Control Rig เป็น gameplay actor

---

# 22. Niagara System Structure

## `NS_CatAura`

```text
NS_CatAura
├─ NE_Aura_Core
├─ NE_Aura_Ring
├─ NE_Aura_Motes
├─ NE_Aura_Shell
└─ NE_Aura_Wisps
```

---

# 23. Aura Layers

## Aura Core

```text
Soft pink volume/glow
follow character
fade in/out
```

## Aura Ring

```text
procedural ring
ground aligned
soft distortion
rotation/pulse
```

## Floating Motes

```text
small glowing particles
floating upward
curl noise
```

## Shell Symbol

```text
procedural spiral/shell
above caster
rotation
pulse
```

## Aura Wisps

```text
spiraling wisps
around body
```

---

# 24. Niagara User Parameters

```text
User.AuraColor
User.AuraRadius
User.AuraIntensity
User.AuraDuration
User.ParticleDensity
User.ShellScale
```

C++ controls them at runtime

---

# 25. Procedural Paw Projectile

นี่คือตัว Hero ของ showcase

ไม่ใช้ sprite sheet เป็น shape หลัก

## `NS_CatPawProjectile`

```text
NS_CatPawProjectile
├─ Paw Core
├─ Paw Edge
├─ Dash Trail
├─ Sparks
└─ Impact
```

---

# 26. Procedural Paw Material

สร้างด้วย math/SDF

Paw ประกอบด้วย:

```text
Main Pad
+
Toe 1
Toe 2
Toe 3
Toe 4
```

ใช้ Circle/Ellipse SDF

Concept:

```text
UV
↓
Center
↓
Distance Field
↓
Main Pad SDF
+
4 Toe SDF
↓
Smooth Union
↓
Paw Mask
```

แล้ว:

```text
Paw Mask
×
Color
×
Glow
→ Emissive
```

---

# 27. Shape Swap

ระบบ Material ไม่ผูกกับ Paw อย่างเดียว

สร้าง:

```text
ShapeType
```

เช่น:

```text
0 Paw
1 Heart
2 Star
3 Shell
4 Fish
5 Spiral
```

User Parameter:

```text
User.ShapeType
```

หรือ Material Parameter

```text
ShapeMode
```

ทำให้โชว์ใน portfolio ได้ว่า:

> Procedural FX system supports runtime shape swapping

---

# 28. Paw Projectile Gameplay Actor

ช่วง prototype:

```text
BP_PawProjectile
```

Components:

```text
Sphere Collision
Niagara Component
Projectile Movement Component
```

Properties:

```text
TargetActor
Speed
HomingAcceleration
EffectSpec
```

Flow:

```text
Spawn
↓
Set Target
↓
Dash
↓
Overlap
↓
Send Effect
↓
Impact FX
↓
Destroy
```

---

# 29. Target Detection

`CatSkillComponent` ทำ:

```text
Sphere Overlap
```

Range:

```text
SkillRange
```

แล้ว filter:

```text
Enemy Tag
หรือ
Interface
หรือ
EffectReceiverComponent
```

ดีที่สุด:

```text
Has EffectReceiverComponent
```

ไม่ต้องเช็ก class EnemyA/B/C

---

# 30. Target Selection

ถ้ามีหลายตัว:

```text
Find all targets
↓
Calculate distance
↓
Pick closest
```

ภายหลังสามารถเปลี่ยน strategy:

```text
Closest
Lowest HP
Highest Threat
Random
Cone
Camera Direction
```

โดยไม่แก้ projectile

---

# 31. Skill Full Runtime Flow

```text
PLAYER PRESSES E
        │
        ▼
CatSkillComponent
        │
        ├── Check Cooldown
        ├── Check State
        └── Find Target
                │
                ▼
          Spawn Aura
                │
                ▼
      OnCatSkillActivated
                │
                ▼
       Spawn Paw Projectile
                │
                ▼
          Dash To Enemy
                │
                ▼
              HIT
                │
                ▼
       EffectReceiverComponent
                │
       ┌────────┼─────────┐
       ▼        ▼         ▼
     Stun     Gold      Carry
       │        │         │
       │        │         ▼
       │        │     Spawn Treat
       │        │         │
       │        │         ▼
       │        │    Attach Hand
       │        │
       └────────┴─────────┐
                          ▼
                  Move To Player
                          │
                          ▼
                      Deliver
                          │
                          ▼
                  Transfer Item
                          │
                          ▼
                   Player Receive
                          │
                          ▼
                     Cat Lick
                          │
                          ▼
                    Consume Item
                          │
                          ▼
                      Skill End
```

---

# 32. Event Architecture

ใช้ events เยอะ ๆ แทนการผูก class กันตรง ๆ

เช่น:

```text
OnSkillActivated
OnTargetAcquired
OnProjectileHit

OnEffectReceived

OnStunStarted
OnStunEnded

OnGoldSpent

OnItemEquipped
OnItemDetached
OnItemTransferred

OnReachedTarget
OnInteractionStarted
OnInteractionFinished

OnTreatReceived
OnTreatLicked
OnTreatConsumed
```

---

# 33. Blueprint ↔ C++ Bridge

C++ ทำ:

```text
logic
state
validation
targeting
effects
data
runtime component
```

Blueprint ทำ:

```text
presentation
animation
SFX
Niagara
timeline
camera
tuning
```

ตัวอย่าง:

```cpp
UFUNCTION(BlueprintImplementableEvent)
void OnCatSkillActivated();
```

Blueprint เอาไป:

```text
Play Animation
Play Sound
Camera Shake
Extra Niagara
```

---

# 34. Folder Structure

```text
Source/MyProject/

Gameplay/
├─ Skills/
│  ├─ CatSkillComponent.h
│  ├─ CatSkillComponent.cpp
│  ├─ CatSkillData.h
│  └─ CatSkillData.cpp
│
├─ Effects/
│  ├─ GameplayEffectTypes.h
│  ├─ GameplayEffectData.h
│  ├─ EffectReceiverComponent.h
│  └─ EffectReceiverComponent.cpp
│
├─ Status/
│  ├─ StatusComponent.h
│  └─ StatusComponent.cpp
│
├─ Economy/
│  ├─ EconomyComponent.h
│  └─ EconomyComponent.cpp
│
├─ Equipment/
│  ├─ AttachmentComponent.h
│  ├─ AttachmentComponent.cpp
│  ├─ CarryItemData.h
│  └─ CarryItemData.cpp
│
└─ Interaction/
   ├─ InteractionComponent.h
   └─ InteractionComponent.cpp
```

Content:

```text
Content/

Blueprints/
├─ Characters/
│  ├─ BP_PlayerCatSkill
│  └─ BP_EnemyTest
│
├─ Projectiles/
│  └─ BP_PawProjectile
│
└─ Items/
   └─ BP_CatTreat

Data/
├─ Skills/
│  └─ DA_CatPawSkill
│
├─ Effects/
│
└─ Items/
   └─ DA_CatTreat

VFX/
└─ CatSkill/
   ├─ Systems/
   │  ├─ NS_CatAura
   │  ├─ NS_CatPawProjectile
   │  ├─ NS_CatPawImpact
   │  └─ NS_EnemyStun
   │
   ├─ Materials/
   │  ├─ M_CatAura_Master
   │  ├─ M_CatPaw_SDF
   │  ├─ M_CatShell_SDF
   │  └─ M_CatTrail
   │
   └─ Functions/
      ├─ MF_SDFCircle
      ├─ MF_SDFEllipse
      ├─ MF_SmoothUnion
      ├─ MF_PawShape
      └─ MF_ShellSpiral

Animations/
├─ Player/
├─ Enemy/
└─ Interaction/
```

---

# 35. Build Order

กูแนะนำ **อย่าทำตาม folder order** ให้ทำตาม vertical slice

## Phase 1 — Input

เสร็จแล้ว:

```text
E
→ CatSkillComponent
```

---

## Phase 2 — Aura

```text
E
→ NS_CatAura
→ attach player
```

---

## Phase 3 — Target

```text
Sphere Overlap
→ closest enemy
```

---

## Phase 4 — Projectile

```text
Spawn Paw Placeholder
→ fly target
```

---

## Phase 5 — Hit

```text
Overlap
→ EffectReceiver
```

---

## Phase 6 — Stun

```text
StatusComponent
→ Stun
```

---

## Phase 7 — Gold

```text
EconomyComponent
→ Spend Gold
```

---

## Phase 8 — Cat Treat

```text
DA_CatTreat
→ spawn
→ attach hand
```

---

## Phase 9 — AI

```text
MoveTo Player
```

---

## Phase 10 — Deliver

```text
Detach enemy
→ attach player
```

---

## Phase 11 — Cat Lick

```text
Animation
→ Notify
→ consume treat
```

---

## Phase 12 — Procedural Paw

แทน placeholder ด้วย:

```text
SDF Material
```

---

## Phase 13 — Niagara Polish

เพิ่ม:

```text
Dash
Trail
Impact
Stun
Aura
```

---

## Phase 14 — Shape Swap

```text
Paw
Heart
Star
Shell
Fish
```

---

# 36. Debug Mode

Portfolio Technical Artist ควรมี debug

สร้าง:

```text
bDebugSkill
```

แล้วโชว์:

```text
Skill Radius
Target Sphere
Selected Target
Projectile Path
Socket Locations
Attachment Slot
Effect Names
State
```

เช่น:

```text
TARGET: Enemy_03
STATUS: STUN
GOLD: -10
ITEM: CatTreat
STATE: Delivering
```

อันนี้ทำให้ project ดู technical ขึ้นเยอะ

---

# 37. Optimization

โชว์ด้วยว่าไม่ได้ทำแค่สวย

## Niagara

```text
GPU particles สำหรับ motes
ต่ำจำนวน particle
fixed bounds
disable unnecessary collision
avoid expensive lights
```

## Projectile

ภายหลัง:

```text
Pooling
```

## FX

ระยะไกล:

```text
ลด spawn rate
ลด trail
ลด motes
```

## Material

ใช้:

```text
procedural math
master materials
parameterized instances
```

แทน texture เยอะ ๆ

---

# 38. Portfolio Presentation

หน้า portfolio ไม่ควรเปิดด้วย architecture diagram

เปิดด้วย:

## Hero Video

```text
E
→ Pink Aura
→ Paw Spawn
→ Dash
→ Impact
→ Enemy Stunned
→ Enemy walks with Cat Treat
→ Gives Cat
→ Cat Licks
```

แล้วค่อย breakdown:

```text
01 Gameplay Result
02 Niagara Breakdown
03 Procedural Paw Shader
04 Shape Swap
05 C++ ↔ Blueprint Architecture
06 Gameplay Effect System
07 Attachment / Item Transfer
08 AI Interaction
09 Debug View
10 Optimization
```

---

# 39. จุดขายของงานนี้

มันไม่ได้เป็นแค่:

> “ทำ Niagara รูปอุ้งเท้าแมว”

แต่มันคือ:

**Realtime Gameplay FX System**

ที่รวม:

```text
Unreal Engine
C++
Blueprint
Niagara
Procedural Material
SDF
Gameplay Events
AI Navigation
Animation
Sockets
Item Attachment
Data-driven Architecture
Optimization
Debugging
```

ซึ่งตรงกับแนว Technical Artist / Realtime VFX มาก

---

# 40. Golden Rule ของ Project นี้

เวลาจะเพิ่มอะไรใหม่ ให้ถามว่า:

```text
นี่เป็น Gameplay?
→ C++

นี่เป็น Data?
→ Data Asset

นี่เป็น Presentation?
→ Blueprint

นี่เป็น FX?
→ Niagara

นี่เป็น Shape / Surface?
→ Material / Shader

นี่เป็น Character Motion?
→ Animation / Control Rig

นี่เป็น AI behavior?
→ AI / Interaction Component
```

แล้วระบบจะไม่พันกัน

---

## Final Target Architecture

```text
                 DATA
                  │
                  ▼
INPUT ──────> SKILL COMPONENT
                  │
                  ▼
             PROJECTILE
                  │
                  ▼
            EFFECT RECEIVER
                  │
       ┌──────────┼──────────┐
       ▼          ▼          ▼
    STATUS     ECONOMY   ATTACHMENT
                               │
                               ▼
                         INTERACTION
                               │
                               ▼
                              AI
                               │
                               ▼
                           ANIMATION

Presentation Layer
───────────────────────────────
Niagara
Material
SFX
Camera
Blueprint Events
```

อันนี้คือ structure ที่กูว่าควร **ล็อกเป็น Master Design** ของสกิลนี้ได้เลย แล้วจากนี้เวลาทำจริงเราก็ไล่ vertical slice ทีละ Phase โดยไม่ต้องเปลี่ยน architecture กลางทาง.