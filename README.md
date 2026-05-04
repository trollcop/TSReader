# TSReaderPro - MPEG Transport Stream Analyzer

**Version 2.8.53b-memorial**

TSReader is a Windows application for analysing MPEG Transport Streams.
Originally developed circa 2004-2008 with Visual C++ 6.0 by Rod Hewitt
KG6TTD (G6TTD), this codebase has been migrated to build with modern
Visual Studio (2022 / 2026) using CMake.

> **In memory of Rod Hewitt KG6TTD (G6TTD)** — passed away March 2025.
> This project continues his work under a memorial build. The running
> application shows a splash screen at startup with memorial text and
> a link to donate via PayPal to help cover hosting costs.

The codebase originally shipped in three editions (Lite, Standard, Pro).
This modernisation project focuses exclusively on the **Pro edition** —
the top-tier version with all features, decoders, and plugins enabled.

## Prerequisites

| Software | Version | Notes |
|----------|---------|-------|
| Visual Studio | 2022 (v17) or 2026 (v18) | Community edition is fine. Install the **Desktop development with C++** workload |
| CMake | 4.3.0+ | Must be on PATH |
| Git | Any recent | For cloning the repo |
| Inno Setup 6 | 6.7+ | Only needed for building the installer |

## Quick Start

```cmd
git clone git@github.com:TSReader/TSReader.git
cd TSReader

:: Generate the Visual Studio solution (Win32/x86 is required)
cmake -G "Visual Studio 18 2026" -A Win32 -B build

:: Build Release
cmake --build build --config Release
```

For Visual Studio 2022, use:

```cmd
cmake -G "Visual Studio 17 2022" -A Win32 -B build
```

> **Note:** The `-A Win32` flag is required. TSReader is a 32-bit (x86) application.

## Build Output

After a successful build, the following files will be in `build\Release\`:

| File | Description |
|------|-------------|
| `TSReaderPro.exe` | Main application (~1.3 MB) |
| `libfaad2.dll` | AAC audio decoder (pre-built, auto-copied) |
| `PEGRPCS.DLL` | Charting library (pre-built, auto-copied) |
| `TSReader_SourceHelper.dll` | Helper library (pre-built, auto-copied) |

## Running

TSReader requires additional runtime files from the source tree. The easiest options:

1. **Use the installer** (recommended) — see *Building the Installer* below.
2. **Run from source directory** — copy the build output to the repo root and run from there.

When the app starts you'll see the memorial splash screen (photo, tribute
text, GitHub and PayPal links, plus a **Continue** button). Click
**Continue** to dismiss it and the normal source-selection / tune flow
begins.

### Required Runtime Files

TSReader loads these from its working directory:

- `*.bmp`, `*.png` — UI icons and status images (plus `rod_splash.png` for the memorial splash)
- `*.ini` — Configuration files
- `*.lst` — Source preset lists
- `Sources\` folder — **Source plugin DLLs** (loaded dynamically via LoadLibrary)
- `Forwarders\` folder — Stream forwarder plugin DLLs
- `Satellites\` folder — Satellite transponder configs

> **Important:** Without the `Sources\` folder, TSReader cannot open any transport streams. At minimum, `TSReader_File.dll` must be present for file-based input.

## Building the Installer

```cmd
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer.iss
```

Output: `installer_output\TSReaderPro_Setup.exe` (~5 MB)

The installer packages everything needed: executable, DLLs, bitmaps,
configs, source plugins, forwarders, satellite data, and the memorial
splash image. The default install location is
`C:\Program Files (x86)\TSReaderPro` (following Windows guidelines for
32-bit applications); the user can change this during setup.

## Recent Changes

- **Memorial splash screen** at startup with Rod's photo, tribute text,
  GitHub link, PayPal donate link, and a Continue button.
- **Cancel in source tune dialog** now returns to the source selection
  dialog rather than exiting the application.
- **Version bumped** to `2.8.53b-memorial` (exe version info and installer).
- **URLs updated** throughout the codebase:
  - `coolstf.com` → `tsreader.co.uk`
  - `rod@coolstf.com` → `support@tsreader.co.uk`
  - GitHub references updated to `github.com/TSReader/TSReader`
- **Purchase link removed** (Digital River checkout URL is dead).
- **Installer** now installs to `C:\Program Files (x86)\TSReaderPro` by
  default with user-selectable directory and admin elevation.

## Project Structure

```
TSReader/
├── CMakeLists.txt          # Main build file
├── installer.iss           # Inno Setup installer script
├── splash.c                # Memorial splash screen module
├── rod_splash.png          # Memorial photo
├── *.c / *.h               # Core TSReader source
├── stubs/                  # SDK replacement implementations
│   ├── isource_impl.c      # ImgSource SDK → stb_image + GDI
│   └── stb_image*.h        # stb single-header image libraries
├── include/                # Shared headers
├── h264/                   # H.264 decoder (built from source)
├── vc1/                    # VC-1 decoder (built from source)
├── libmad-0.15.1b/         # MPEG audio decoder (built from source)
├── a52dec-0.7.4/           # AC-3 audio decoder (built from source)
├── libmpeg2/               # MPEG-2 video decoder (pre-built .lib)
├── Sources/                # Source plugin DLLs (pre-built)
├── Forwarders/             # Forwarder plugin DLLs (pre-built)
└── Satellites/             # Satellite config INI files
```

## Troubleshooting

**CMake can't find a compiler** — Run from a Developer Command Prompt, or ensure VS C++ workload is installed.

**Missing DLLs at runtime** — Check that `libfaad2.dll`, `PEGRPCS.DLL`, and `TSReader_SourceHelper.dll` are alongside the exe. The build copies these automatically.

**No source plugins** — Ensure the `Sources\` subfolder with plugin DLLs is in the same directory as the executable.

**Splash photo not showing** — Ensure `rod_splash.png` is in the install directory (next to `TSReaderPro.exe`). The splash falls back to text-only if the image cannot be loaded.

**Clean rebuild:**

```cmd
rmdir /s /q build
cmake -G "Visual Studio 18 2026" -A Win32 -B build
cmake --build build --config Release
```
