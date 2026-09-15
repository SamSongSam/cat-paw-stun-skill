# Setup Notes — สิ่งที่ต้องเตรียมก่อนทำต่อบนคอมเครื่องใหม่

## 1. ติดตั้งบนเครื่องใหม่

- **Git** — https://git-scm.com/downloads
- **GitHub CLI (gh)** — `winget install GitHub.cli` แล้ว `gh auth login` (เลือก GitHub.com → HTTPS → login ผ่านเบราว์เซอร์ ใช้บัญชี `SamSongSam`)
- **Unreal Engine 5.x** — ต้องติดตั้งเวอร์ชันที่ตรงกับโปรเจกต์จริงที่จะเอาโค้ดนี้ไปวาง (ดูข้อ 4)
- Visual Studio 2022 (Desktop development with C++ + Game development with C++ workload) — สำหรับ compile C++ module ของ UE

## 2. Clone repo

```bash
git clone https://github.com/SamSongSam/cat-paw-stun-skill.git
```

## 3. สิ่งที่มีอยู่แล้วใน repo นี้ (อ่านตามลำดับ)

1. [CLAUDE.md](CLAUDE.md) — ภาพรวมโปรเจกต์ + สถานะปัจจุบัน (Phase 1-2 เท่านั้น) + code style ที่ต้องยึด
2. [CatSkill_test.md](CatSkill_test.md) — โครงสร้างเต็มของสกิล (40 sections) ใช้เป็น reference ตลอดทั้งโปรเจกต์
3. `Source/MyProject/` — โค้ด C++ ที่เขียนไว้แล้ว (CatSkillData, CatSkillComponent, CatPlayerCharacter)
4. [Source/MyProject/INTEGRATION_NOTES.md](Source/MyProject/INTEGRATION_NOTES.md) — ขั้นตอน copy โค้ดเข้าโปรเจกต์จริง (module macro, Build.cs, asset ที่ต้องสร้างเอง)
5. `CatAura_SafeBuild.py`, `CatSkill_AutoWire.py` — สคริปต์รันในตัว UE Python console

## 4. สิ่งที่ผม (Claude) ยังไม่รู้ — ต้องบอกตอนเปิดคอมใหม่หรือใส่ไว้ในนี้ก่อนก็ได้

กรอกด้านล่างนี้แล้ว commit กลับมา หรือบอกในแชทตอนเปิดเซสชันใหม่ก็ได้ — จะได้ไม่ต้องเดา/ถามซ้ำ:

- [ ] **มี `.uproject` อยู่แล้วหรือยัง?** ถ้ามี: path คืออะไร, engine version เท่าไหร่ (5.3 / 5.4 / 5.8 ...), ชื่อ module หลักคืออะไร (จะได้รู้ว่า `MYPROJECT_API` ต้องเปลี่ยนเป็นอะไร)
- [ ] ถ้า**ยังไม่มี** `.uproject`: จะสร้างโปรเจกต์ใหม่จาก template ไหน (Third Person / Blank) engine version อะไร
- [ ] โปรเจกต์นี้ใช้ **Gameplay Ability System (GAS)** อยู่แล้วหรือเปล่า? (CatSkill_test.md เขียนสมมติว่ายังไม่ใช้ GAS แต่ถ้าจริงๆ มีอยู่แล้ว โครงสร้าง Effect Receiver ทั้งหมดควรผูกกับ GAS แทนที่จะทำระบบขนาน)
- [ ] มี Niagara System ตัวอย่าง/template ที่ใช้งานได้อยู่แล้วในโปรเจกต์ไหม (สำหรับให้ `CatAura_SafeBuild.py` เลือกเป็น template ตอน duplicate)
- [ ] ชื่อ/ตำแหน่ง Character Blueprint ที่ใช้อยู่ตอนนี้ (ถ้ามีอยู่แล้ว จะได้รู้ว่าจะ reparent เป็น `ACatPlayerCharacter` ยังไงโดยไม่พังของเดิม)

## 5. ขั้นตอนถัดไปเมื่อพร้อม (ทำตามลำดับ ไม่ข้าม)

1. Copy `Source/MyProject/*` เข้า `<YourProject>/Source/<YourModule>/`
2. แก้ `MYPROJECT_API` → `<YOURMODULE>_API` ทุกไฟล์
3. เพิ่ม `EnhancedInput`, `Niagara` ใน `PublicDependencyModuleNames` ของ `<YourModule>.Build.cs`
4. Regenerate project files → compile
5. เปิด Editor → สร้าง `IMC_Default` / `IA_CatSkill` (ถ้า `CatSkill_AutoWire.py` สร้างให้ไม่ได้เพราะ factory ไม่รองรับ)
6. รัน `CatAura_SafeBuild.py` (เลือก Niagara System ต้นแบบก่อนรัน)
7. รัน `CatSkill_AutoWire.py`
8. Set `BP_PlayerCatSkill` เป็น Default Pawn Class แล้วกด Play ทดสอบกด E

## Git remote

Repo นี้ push ไว้ที่ https://github.com/SamSongSam/cat-paw-stun-skill แล้ว (public) — เครื่องใหม่ clone ตรงนี้ได้เลย ไม่ต้อง push ซ้ำจากเครื่องเก่า
