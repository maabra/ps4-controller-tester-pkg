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

The diagnostic model is deliberately simple: each controller gets a stable
slot and sampled state, ready for a fullscreen framebuffer or SDL-PS4 view.
The current source focuses on the firmware-facing pad backend; the renderer
must be linked from the graphics layer selected for your OpenOrbis toolchain.

## Build

This repository contains the app source and packaging recipe, not a bundled
Sony SDK or a pre-signed package. Install the OpenOrbis toolchain separately
and provide its path through `OPENORBIS_ROOT`.

From a shell with Docker available:

```powershell
$env:OPENORBIS_ROOT = 'C:\path\to\OpenOrbis-Toolchain'
docker build -t ps4-controller-tester-build .
docker run --rm -v "${PWD}:/src" -v "${env:OPENORBIS_ROOT}:/opt/OpenOrbis:ro" ps4-controller-tester-build pkg
```

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
src/pad_backend.h   OpenOrbis pad adapter boundary
Makefile            ELF and package targets
Dockerfile          reproducible OpenOrbis build environment hook
param.sfo.in        package metadata template
```

## Controls

The backend samples all four slots at 60 Hz. Map `ORBIS_PAD_BUTTON_OPTIONS` to
quit in the renderer/event loop. Missing controllers remain represented by
empty slots so all four ports can be checked at once.
