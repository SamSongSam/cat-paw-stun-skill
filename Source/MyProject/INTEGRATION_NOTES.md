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
});
```

ถ้าไม่เพิ่มสองตัวนี้ compile จะ error หา header `EnhancedInputComponent.h` / `NiagaraFunctionLibrary.h` ไม่เจอ

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

## สงสัย/ติดตรงไหนบอกได้เลย

ถ้า compile แล้ว error หรือ module ไม่ตรงกับที่โปรเจกต์จริงตั้งไว้ (เช่นใช้ GAS อยู่แล้ว, ชื่อ module ไม่ใช่ MyProject) บอกมาได้เลยจะปรับให้ตรง
