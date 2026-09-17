# Integration Notes — Phase 1-2 (Input → Aura)

โค้ดนี้เขียนแยกลอยไว้ที่ `D:\2\cat\Source\MyProject\` ยังไม่ได้ผูกกับ `.uproject` จริง — ตามที่คุยกันไว้ว่าจะ copy เข้าโปรเจกต์จริงทีหลัง ก่อน copy ต้องทำตามนี้ก่อนถึงจะ compile ผ่าน:

## 1. เปลี่ยนชื่อ module macro

ทุกไฟล์ใช้ `MYPROJECT_API` เป็น placeholder — ต้องเปลี่ยนเป็น macro จริงของโปรเจกต์คุณ (Unreal gen ให้อัตโนมัติ ชื่อตาม module เช่นถ้า module ชื่อ `CatGame` จะเป็น `CATGAME_API`) หา/แทนที่ทั้งหมดก่อน compile

## 2. เพิ่ม module dependency ใน `<YourModule>.Build.cs`

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "EnhancedInput",   // ต้องมี — ใช้ใน CatPlayerCharacter.cpp
    "Niagara",         // ต้องมี — ใช้ใน CatSkillComponent.cpp / CatSkillData.h
    "GameplayTags",    // ต้องมี — ใช้ใน GameplayEffectTypes.h / CatGameplayTags.h/.cpp
});
```

ถ้าไม่เพิ่มตัวพวกนี้ compile จะ error หา header `EnhancedInputComponent.h` / `NiagaraFunctionLibrary.h` / `NativeGameplayTags.h` ไม่เจอ

## 3. Asset ที่ต้องสร้างในตัว Editor (โค้ดสร้างให้ไม่ได้ เพราะเป็น .uasset ไม่ใช่ text)

| Asset | ประเภท | ใช้ที่ไหน |
|---|---|---|
| `IMC_Default` | Input Mapping Context | `ACatPlayerCharacter::DefaultMappingContext` |
| `IA_CatSkill` | Input Action (Digital/bool) | `ACatPlayerCharacter::CatSkillAction`, map เข้า `IMC_Default` โดยผูกปุ่ม E |
| `DA_CatPawSkill` | Data Asset ชนิด `UCatSkillData` | `UCatSkillComponent::SkillData` (ตั้งค่า Cooldown, SkillDisplayName) |
| `NS_CatAura` | Niagara System | `DA_CatPawSkill.AuraSystem` — ใช้ template ที่มีอยู่แล้วก่อน (ตามที่ `CatAura_SafeBuild.py` ทำ — duplicate จาก template ที่เลือกไว้) |
| `BP_PlayerCatSkill` | Blueprint, Parent Class = `ACatPlayerCharacter` | ใส่ mesh/camera/spring arm — ห้ามใส่ gameplay logic ที่นี่ |

## 4. Cooldown formula ที่ใช้จริงใน `CanActivateSkill()`

```cpp
(World->GetTimeSeconds() - LastCastTime) >= SkillData->Cooldown
```

- `LastCastTime` เริ่มที่ `-1.0f` เป็นค่า sentinel แปลว่า "ยังไม่เคยแคสต์" → การเช็คแรกจะผ่านเสมอไม่ว่าคูลดาวน์เท่าไหร่ (เช็ค `LastCastTime < 0.0f` ก่อนแยกไว้ ไม่ปนกับสูตรลบเวลา เพราะถ้าใช้ `0.0f` เป็น sentinel แล้วเผลอ activate ตอน `GetTimeSeconds()` ยังน้อยกว่า cooldown จริงๆ จะเข้าใจผิดว่าคูลดาวน์ยังไม่หมด)
- ครั้งถัดไป: เอาเวลาปัจจุบันลบเวลาที่แคสต์ล่าสุด ถ้า **มากกว่าหรือเท่ากับ** ค่า Cooldown ถึงจะแคสต์ได้อีก

## 5. Flow ที่ implement แล้ว (Phase 1-2 เท่านั้น)

```
กด E
→ ACatPlayerCharacter::HandleCatSkillInput
→ UCatSkillComponent::ActivateCatSkill
→ CanActivateSkill() เช็ค SkillData / bIsSkillActive / cooldown
→ SpawnAura() — UNiagaraFunctionLibrary::SpawnSystemAttached
→ OnCatSkillActivated() (BlueprintImplementableEvent — ไปเล่น anim/sound ใน BP)
```

**ยังไม่ทำ**: target detection, projectile, hit/stun, gold, carry item, AI, cat lick — เป็น Phase 3 เป็นต้นไปตาม `CatSkill_test.md` section 35 จะเพิ่มทีหลังทีละ phase

## 5.1 Bug ที่แก้แล้ว: `DA_CatPawSkill.ProjectileSpeed` เคยไม่มีผลอะไรเลย

ก่อนหน้านี้ `UCatSkillData::ProjectileSpeed` เป็นแค่ field ที่แก้ใน Editor ได้ แต่ไม่มีโค้ดไหนอ่านมันเลย — ความเร็วจริงของ projectile มาจากค่า hardcode ใน `ACatPawProjectile()` constructor (`InitialSpeed = 2000.0f`) เสมอ ไม่ว่า Data Asset จะตั้งค่าอะไรไว้ก็ตาม (ถ้าทำสอง skill ที่ตั้ง ProjectileSpeed ต่างกัน ปาก็จะเร็วเท่ากันอยู่ดี)

ตอนนี้แก้แล้ว: `CatSkillComponent::SpawnProjectile` ส่ง `SkillData->ProjectileSpeed` เข้า `InitializeProjectile(...)` ทุกครั้งที่ spawn — ความเร็วจริงมาจาก Data Asset แล้ว ค่าใน constructor (`2000.0f`) เหลือไว้แค่เป็นค่า preview ตอนลาก `BP_PawProjectile` ไปวางในระดับเปล่าๆ โดยไม่ผ่าน skill

## 6. Effect Type เป็น Gameplay Tag แล้ว ไม่ใช่ C++ enum

`FGameplayEffectSpec::Type` (เดิมเป็น `enum class EGameplayEffectType`) เปลี่ยนเป็น `FGameplayTag` — ตอนนี้ authoring จาก Editor ล้วนๆ:

- ใน Data Asset (`DA_CatPawSkill.Effects[]`) หรือที่ไหนก็ตามที่มี `FGameplayEffectSpec` field ให้กรอก จะเห็น **tag picker** (ต้นไม้ + ช่องค้นหา) แทน dropdown enum เดิม — เลือกจาก `Effect.Stun`, `Effect.GoldCost`, `Effect.CarryItem`, `Effect.MoveToActor`, `Effect.TransferItem`
- Tag 5 ตัวนี้ประกาศแบบ native ใน [CatGameplayTags.h](Gameplay/Effects/CatGameplayTags.h) / `.cpp` เพราะเป็นตัวที่ `EffectReceiverComponent::ReceiveEffect` มี logic รองรับจริง — ไม่ต้องไปสร้าง `.ini` เอง จะขึ้นให้อัตโนมัติใน **Project Settings > Gameplay Tags**
- ถ้าจะเพิ่ม effect type ใหม่ (เช่น `Effect.Burn`): สร้าง tag ใหม่ได้จาก **Project Settings > Gameplay Tags > Add New Gameplay Tag** โดยไม่ต้องแตะโค้ดเลย — แต่ตัว behavior ยังต้องมีคนเพิ่ม `else if (Type == ...)` ใน `EffectReceiverComponent.cpp` อยู่ดี (dispatch logic เป็น Gameplay → C++ ตาม Golden Rule ของโปรเจกต์ ทำให้ data-only ทั้งหมดไม่ได้) — ถ้าลืมเพิ่ม จะไม่ crash แค่ log warning `unrecognized effect Type tag` แล้วไม่ทำอะไร

## 7. Dynamic FX — Niagara User Parameters ที่ต้องสร้างเอง (NEXT.md)

C++ ฝั่ง Gameplay push ค่าพวกนี้เข้า Niagara ผ่าน `UNiagaraComponent::SetVariable*` (`FName` คงที่อยู่ใน [CatFXParamNames.h](Gameplay/Effects/CatFXParamNames.h)) — ถ้า User Parameter ชื่อ/ชนิดไม่ตรงกับที่ Niagara System เอง define ไว้ การ set จะเงียบๆ ไม่มี error แค่ไม่เกิดอะไรขึ้น ต้องสร้างเองใน Editor ให้ตรงชื่อ/ชนิดเป๊ะๆ ดังนี้:

| Niagara System | User Parameter | ชนิด | ใครเขียนค่า | Set เมื่อไหร่ |
|---|---|---|---|---|
| `NS_CatAura` (`DA_CatPawSkill.AuraSystem`) | `StunStack` | int32 | `CatSkillComponent::SpawnAura` | ทันทีหลัง spawn aura (อ่านจาก target ปัจจุบัน ถ้ายังไม่มี target = 0) |
| `NS_CatAura` | `FXIntensity` | float (0-1) | `CatSkillComponent::SpawnAura` | เดียวกับด้านบน — `StunStack / 4` |
| Trail/main paw system (ตั้งใน `BP_PawProjectile`'s `NiagaraComponent`) | `TargetPosition` | Vector | `CatPawProjectile::InitializeProjectile` | ตอน spawn projectile (fix ค่าเดียวตลอด flight ตาม section 5) |
| Trail system | `DistanceToTarget` | float | เดียวกับด้านบน | เดียวกับด้านบน |
| Trail system | `AttackVelocity` | Vector | เดียวกับด้านบน | เดียวกับด้านบน |
| `DA_CatPawSkill.ImpactSystem` | `HitLocation` | Vector | `CatPawProjectile::SpawnImpactFX` | ตอน hit ก่อน `Activate()` |
| `ImpactSystem` | `HitNormal` | Vector | เดียวกับด้านบน | เดียวกับด้านบน |
| `ImpactSystem` | `StunStack` | int32 | เดียวกับด้านบน | เดียวกับด้านบน (stack **หลัง** hit นี้ ไม่ใช่ก่อน) |
| `ImpactSystem` | `FXIntensity` | float (0-1) | เดียวกับด้านบน | เดียวกับด้านบน |

หมายเหตุ:
- `StunStack` มาจาก `UStatusComponent::GetStunStack()` บน**ตัวที่โดนตี** (ไม่ใช่ตัวแมว) — เพดานเป็นค่า `EditDefaultsOnly` (`UStatusComponent::MaxStunStack`, default = 4, ปรับได้ต่อ Blueprint เช่นบอสจะให้เพดานสูงกว่าศัตรูทั่วไปก็ได้) วน 0→1→2→3→4→reset อัตโนมัติ: `RemoveStun()` จะ reset stack กลับเป็น 0 เองตอนที่ stun ตัวที่ทำให้ stack ถึงเพดานหมดเวลาไปแล้ว (ไม่ reset ทันทีตอนโดนตีครั้งที่ 4 — ถ้า reset ทันทีจะทำให้ impact FX/gold ของการตีครั้งที่ทำให้ stack เต็มดันอ่านเห็นค่า 0 แทนที่จะเป็น 4)
- ค่า `GoldCost` effect ถูก scale ตาม stack แล้ว: `CatSkillComponent::BuildEffectSpecs` คูณ `Magnitude` ของทุก effect ที่ type เป็น `Effect.GoldCost` ด้วย "stack ที่กำลังจะเป็นหลังจากตีครั้งนี้" (1x ตีแรก, 2x ตีที่สอง, ... สูงสุดตาม `MaxStunStack`) — ตั้งค่า `Magnitude` ใน Data Asset เป็นค่า**ฐาน** (ตีแรก) เท่านั้น ไม่ต้องคูณเผื่อเอง
- ทุก emitter/graph ที่จะ react กับพารามิเตอร์พวกนี้ต้อง bind เอง (ผูก scale/velocity/color เข้ากับ User Parameter) — โค้ด C++ แค่ set ค่าให้ ไม่ได้สร้าง node ใน graph ให้

## สงสัย/ติดตรงไหนบอกได้เลย

ถ้า compile แล้ว error หรือ module ไม่ตรงกับที่โปรเจกต์จริงตั้งไว้ (เช่นใช้ GAS อยู่แล้ว, ชื่อ module ไม่ใช่ MyProject) บอกมาได้เลยจะปรับให้ตรง
