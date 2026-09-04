# PS4 Controller Tester

A small OpenOrbis homebrew application for checking up to four DualShock 4
controllers on a jailbroken PS4. The app is intended for firmware 11.02 and
does not use firmware-private exploits or payloads. It only reads controller
state through the public homebrew pad interface.

## What it tests

- USB and Bluetooth controller connection state
- Every face, shoulder, stick-click, Share, Options, PS, and touch-pad button
- Both analog sticks and the L2/R2 axes
- Per-controller battery and connection metadata when exposed by the SDK
- Four controllers at the same time

The app opens a fullscreen 1280x720 logical canvas and scales it to the PS4
output. It shows four stable controller panels with connection status, both
stick positions, L2/R2 pressure bars, face-button indicators, and a live drift
check. A small built-in bitmap font keeps the UI self-contained.

## Build

This repository contains the app source and packaging recipe, not a bundled
Sony SDK or a pre-signed package. Use the official OpenOrbis toolchain
container and provide a built SDL2-PS4 library under `third_party/SDL-PS4`.

From a shell with Docker available:

```powershell
docker pull openorbisofficial/toolchain:latest
docker run --rm -w /workspace -v "${PWD}:/workspace" openorbisofficial/toolchain:latest make pkg
```

On Windows, the local prerequisites are GNU Make, LLVM/Clang, Python, CMake,
Ninja, Git, and Docker Desktop. They are installed through Scoop or already
available on the system. If `docker info` reports a Linux engine error, restart
Docker Desktop and make sure its WSL 2 backend is enabled before running the
commands above.

The build produces `out/PS4ControllerTester.elf` and, when the package tools
are installed, `out/PS4ControllerTester.pkg`. The package must be installed
only on a PS4 you own and have legally jailbroken. A package built for this
project is not a jailbreak or a firmware bypass.

## Firmware note

The code targets the OpenOrbis SDK API level used by firmware 11.02. The pad
state structures are kept behind `src/pad_backend.h` so an SDK update can be
adapted in one place. No application can guarantee compatibility with every
custom jailbreak payload; test the resulting package on the exact 11.02
environment before distributing it.

## Project layout

```text
src/main.c          application loop and controller polling
src/tester.c        button, trigger, and stick diagnostics
src/tester.h        diagnostics API
src/renderer.c      fullscreen SDL2 UI and built-in bitmap font
src/renderer.h      renderer API
src/pad_backend.h   OpenOrbis pad adapter boundary
Makefile            ELF and package targets
Dockerfile          reproducible OpenOrbis build environment hook
param.sfo.in        package metadata template
```

## Controls

The backend samples all four slots at 60 Hz. Hold `Options` on any connected
controller to quit. Missing controllers remain represented by empty slots so
all four ports can be checked at once.
