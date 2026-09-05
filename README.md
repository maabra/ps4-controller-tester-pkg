# PS4 Hello World Homebrew

A minimal, stable OpenOrbis homebrew application for PlayStation 4 hardware. It initializes hardware video output directly via `SceVideoOut` with double buffering at 1080p 60 FPS, initializes `SceUserService` and `ScePad` for DualShock 4 input diagnostics, and uses a built-in bitmap font with zero external runtime dependencies.

## Features

- **Direct Framebuffer Rendering**: Native `SceVideoOut` 1920x1080 @ 60 FPS with VSync flipping.
- **Zero Heavy Runtime Dependencies**: No SDL2 or OpenGL/Piglet requirements that can cause launch crashes on hardware.
- **Built-in 8x8 Bitmap Font**: Embedded ASCII font rendering for crystal clear diagnostic text.
- **Hardware Controller Polling**: Proper `SceUserService` user initialization and `ScePad` reading with live button matrix, analog sticks (LX/LY, RX/RY), and connection status.
- **Smooth 60 FPS Test Animation**: Moving indicator bar to verify smooth, un-throttled frame rendering in real time.

## Build

### Using GitHub Actions CI
Push changes to `main` or trigger the **Build PS4 package** workflow in GitHub Actions. It will compile the code, construct the fself eboot and param.sfo, build the `.pkg` file with `PkgTool.Core`, and publish the ready-to-install `.pkg` artifact.

### Using Docker locally
```powershell
docker pull openorbisofficial/toolchain:latest
docker run --rm -w /workspace -v "${PWD}:/workspace" openorbisofficial/toolchain:latest make pkg
```

The output package will be generated at `out/IV0000-HELO00001_00-HELLOWORLD000000.pkg` (or `out/PS4HelloWorld.pkg`).

## Project Layout

```text
src/main.c          Complete standalone Hello World, VideoOut framebuffer & Pad polling
Makefile            OpenOrbis ELF and PKG build rules
param.sfo.in        Package metadata template
package-assets/     Icon asset (icon0.png) for PS4 Home menu
.github/workflows/  Automated CI pipeline to build & package the PKG
```
