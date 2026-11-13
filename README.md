# GTA3 Multiplayer (Android)

Multiplayer modification for the official Android port of **Grand Theft Auto III**
(`com.rockstar.gta3`, version 1.9, arm64-v8a).

The mod is a native library `libmultiplayer.so` that is loaded by the game
(`System.loadLibrary("multiplayer")` in `GTA3.smali`) and hooks into the
already-running `libR1.so` in memory. No game files are patched on disk —
everything happens at runtime with [Dobby](https://github.com/jmpews/Dobby).

---

## What works right now

| Feature | Status |
|---|---|
| Library injected into the game | ✅ works |
| Dobby hook installation (`libR1.so` offsets) | ✅ works |
| Real **MULTIPLAYER** item in the main menu | ✅ works |
| Tap detection (opens MP, no accidental trigger on scroll) | ✅ works |
| Config loading from `3mp_config.txt` | ✅ works |
| Chat / message window in game | ✅ works |
| RakNet client, connect to server | ⚠️ connects only if a live server exists |
| Game launch in MP mode (`DoSettingsBeforeStartingAGame`) | ✅ works |
| Loading `main_mp.scm` instead of `main.scm` | ⚠️ loads, but crashes on `CCamera::UpdateTargetEntity` |
| Texture streaming with the custom script | ❌ broken (see Known issues) |
| Player spawn created from native code | ❌ not implemented |
| Sync of remote players | 🚧 stubs only |

---

## Building

### Requirements

* Android NDK **27.2.12479018** (or newer r27)
* Windows host with Git Bash, or Linux/macOS
* A GTA3 1.9 APK decompiled to a folder (only `libR1.so` is needed for offsets)

### Command

```bash
cd /c/gta3-multiplayer-andoid-main/jni
NDK="/c/Users/$USER/AppData/Local/Android/Sdk/ndk/27.2.12479018"

"$NDK/prebuilt/windows-x86_64/bin/make.exe" \
  -f "$NDK/build/core/build-local.mk" \
  NDK_PROJECT_PATH=C:/gta3-multiplayer-andoid-main \
  SHELL=cmd -j4
```

Output:

```
libs/arm64-v8a/libmultiplayer.so
```

### Install

The library must be placed inside the game's APK, in `lib/arm64-v8a/`, and the
APK must be re-signed. The game loads it automatically on start.

---

## Configuration

The mod reads its settings from a plain text file inside the game data folder:

```
/sdcard/Android/data/com.rockstar.gta3/files/GTA3/3mp_config.txt
```

Format:

```
nick=Alexander
ip=your.server.ip
port=5555
```

If the file is missing, built-in defaults are used and a warning is logged.
The mod never creates the file itself — Android 11+ forbids writes to `/sdcard`
from the game process, but reading an existing file is allowed.

---

## Architecture

```
jni/
├── main.cpp              JNI entry (JNI_OnLoad), starts the init thread
├── main.h                shared globals, touch token
├── game/
│   ├── game.h            libR1.so offsets + typed function pointers
│   └── hooks.cpp         all Dobby hooks (menu, script, files, rendering)
├── client/
│   ├── Multiplayer.cpp   CMultiplayer: lifecycle, config, Process() tick
│   ├── CNetClient.cpp    RakNet client, packet dispatch
│   ├── CLocalPlayer.cpp  local player sync (mostly stub)
│   ├── CRemotePlayer.cpp remote player representation (stub)
│   ├── CVehicleRegistry.cpp vehicle tracking
│   ├── keyboard.cpp      ImGui on-screen keyboard (Android input)
│   └── NetProtocol.h     packet IDs and structs
├── gui/
│   ├── gui.cpp           ImGui backend
│   ├── CChatWindow.cpp   in-game chat
│   ├── CSpawnScreen.cpp  spawn selection screen
│   ├── ServerList.cpp    server browser (currently disabled)
│   └── CDialogWindow.cpp modal dialogs
├── util/                 ARM helpers, logging, memory utilities
└── vendor/               Dobby, ImGui, RakNet
```

### How the menu item is added

The game keeps its front-end screens in a table reachable at
`*(void**)(libR1_base + 0x46DE68)`. Each screen is `0x19C` bytes, items start at
`+0x48` with a stride of `0x14`; an item is `{action:int, text:char[8]}` where
`text` is a GXT key.

The mod inserts `{0, "MP_STA"}` into the first free slot of screen 51 and
provides an icon for `MP_STA` by delegating to `GetIconForOption("FEN_STA")`,
falling back to `FEP_STA` and then `FET_OPT`.

A tap is detected by comparing an incrementing token (`g_menuTapToken`) that is
bumped only on `ACTION_UP`, so scrolling past the item does not trigger it.

### Offsets

All hook addresses were taken from `llvm-objdump -T` (`.dynsym`, real code), not
from the PLT stubs. Using PLT addresses caused `DobbyHook+164` to SIGSEGV on
startup.

| Symbol | Offset |
|---|---|
| `GetIconForOption` | `0x221464` |
| `Render2dStuff` | `0x220800` |
| `Render2dStuffAfterFade` | `0x220B60` |
| `ProcessButtonPresses` | `0x22C08C` |
| `CText::Get` | `0x2FDDC0` |
| `DoSettingsBeforeStartingAGame` | `0x22E538` |
| `CFileMgr::OpenFile` | `0x21BA30` |
| `CFileMgr::Read` | `0x21BAE8` |
| `CFileMgr::Seek` | `0x21BB20` |
| `CFont::PrintString` | `0x2BDF80` |
| `CFont::SetScale` | `0x2BD1C8` |
| `CFont::SetColor` | `0x2BD1F8` |
| `CFont::SetFontStyle` | `0x2BD2F0` |
| `CFont::SetCentreOn` | `0x2BF9D8` |
| `CFont::SetCentreOff` | `0x2BD260` |
| `FindPlayerPed` | `0x241BA0` |
| `CCamera::UpdateTargetEntity` | `0x208A6C` |

---

## Known issues

### 1. Custom script crashes the camera

`main_mp.scm` is a Sanny Builder 3.6.2 script written for **re3**. GTA3's
`CREATE_PLAYER` opcode does not behave the same, so the player ped is never
created. `CGame::Initialise` then calls `CCamera::UpdateTargetEntity`, which
calls `FindPlayerPed()` and dereferences `NULL`:

```
#00 CCamera::UpdateTargetEntity()+236
#01 CCamera::Process()+116
#02 CGame::Initialise(char const*)+1048
```

Current state: script substitution is **disabled**, the game loads the original
`main.scm` and everything works normally.

### 2. File substitution must not touch every read

`CFileMgr::Read` is called for **all** game files, not just the script. An early
version returned the MP script for every read, which corrupted all model and
texture data — the world rendered as flat grey polygons with no textures.

The fix: only substitute a single block whose size is at least 100 KB
(`SIZE_MAIN_SCRIPT` = 512 KB), zero-fill the rest of that block, and leave every
other read untouched.

### 3. Dead server address

The default address `168.113.187.38:5555` is unreachable (100% packet loss, TCP
port closed). An end-to-end connection cannot be tested without a live server.
Set your own address in `3mp_config.txt`.

### 4. Heavy source files are still stubs

`CNetClient.cpp`, `CRemotePlayer.cpp` and `CLocalPlayer.cpp` in this repository
are much smaller than their counterparts in the original re3 multiplayer source.
They pull in heavy re3 headers (`PlayerPed.h`, `World.h`, `Streaming.h`) that do
not exist in this build, so they need an offset-based rewrite before real sync
will work.

---

## Development history

The mod was built incrementally. Each major step is a separate commit so the
reasoning behind every hook is preserved.

---

## Legal

This is a fan-made modification for personal use. Grand Theft Auto III is
property of Rockstar Games / Take-Two Interactive. No game assets are
distributed here — the repository contains only original source code and
third-party libraries (Dobby, ImGui, RakNet) under their respective licenses.
