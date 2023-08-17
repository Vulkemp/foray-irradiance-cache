# Foray Irradiance Cache

[![Irradance Cache Example](docs/irradiancecache.jpg)](https://www.youtube.com/watch?v=4QpiDKBqSW4)

[Short video showcase](https://www.youtube.com/watch?v=4QpiDKBqSW4)

A world space irradiance cache implemented by [@Firestar99](https://github.com/Firestar99) based upon the foray framework.

## Setup

```
git clone --recursive https://github.com/Vulkemp/foray-examples
```
`--recursive` clones the required submodule `foray`. Then import it into your IDE as a cmake project, or use cmake directly. Run the `irradiance-cache` binary for the main executable. Pre-build binaries are not available as the project is not setup to run outside a development environment.

## Tested Build Environments
* Linux RADV
* Windows Nvidia
* sadly AMDVLK / AMD on Windows does not work, it segfaults inside the driver stack during raytrace pipeline initialization

## Controls
* WASD: standard movement
* Shift/Ctrl: move up / down
* space: lock/unlock mouse to rotate camera
* C: clear irradiance cache
* T/G: previous / next Mode

## Settings
* Traces per Frame: how many traces should be sent out per frame and probe, higher values trade worse performance for faster accumulation
* Accumulation Quality: adjusts accumulation factor to match traces per frame so the image quality remains mostly constant, higher values trade worse accumulation speed for better final image quality
* Accumulation Factor: the percentage by which the newly sampled values should be mixed with the previous values, higher values trade accumulation speed for better final image quality
* Normal Offset Factor: When sampling some geometry, the light will be sampled from the irradiance cache at the position the geometry was hit plus the normal of the geometry times this factor
* Alow skip IC: If a mode does not use the irradiance cache (IC) allow it to skip the accumulation stages, used to estimate overhead introduced by running stages doing nothing

Settings in code `examples/irradiance-cache/src/IrradianceCacheApp.cpp`:
* SCENE_FILE: path to the gltf scene file from the `/data` directory
* PROBE_DISTANCE: the distance between individual probes of the irradiance cache, with the size of the scene will determine the probe dimensions
* INVERT_BLIT_INSTEAD: If true, will invert the viewport when blitting. If false, will invert the scene while loading to -Y up.
* USE_CAMERA_FROM_GLTF: instead of an interactable camera load it from the gltf, useful for comparison against other renderers.

