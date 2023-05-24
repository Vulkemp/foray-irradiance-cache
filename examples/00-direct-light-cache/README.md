# Irradiance Cache example
ic: Irradiance Cache

## Controls
* WASD: movement
* Space: capture mouse
* T, G: decrement / increment rendering mode, cycling though:
  * INDIRECT_TRACE_DIRECT_TRACE: trace everything
  * INDIRECT_IC_DIRECT_TRACE: ic indirect, raytrace direct **default**
  * INDIRECT_IC: only ic indirect, part #1 of default
  * DIRECT_TRACE: only direct raytrace, part #2 of default
  * DIRECT_IC: direct illumination though ic
  * DEBUG_PATTERN: a debug pattern repeating every 10 probes
* C: clear irradiance cache, also happens when switching modes

## Code
The type [IrradianceCache](src/IrradianceCache.h) manages all the resources required for the Irradiance Cache.

## Stages
### [IrradianceCacheDirectStage](src/IrradianceCacheDirectStage.h)
```
direct illumination = raycast(texel center to lights)
tempImage[pixel] = indirectImage[pixel] + direct illumination
```
Takes the old indirect light 
### [IrradianceCacheIndirectStage](src/IrradianceCacheIndirectStage.h)
```
uniform temporalSpeed = [0.85 0.98]
accumLight = 0
for (N times) {
    nearbySurface = raycast(texel center to random direction)
    accumLight += sampleIrradianceCache(tempImage, nearlySurface) * material(nearbySurface)
}
indirectImage[pixel] = lerp(accumLight, tempImage[pixel], temporalSpeed)
```
### [FinalRTStage](src/FinalRTStage.h)
```
surface = raycast(camera to pixel direction)
direct illumination = raycast(surface to lights)
indirect illumination = sampleIrradianceCache(indirectImage, surface)
outImage = direct illumination + indirect illumination
```
### SwapCopyStage
blit outImage to swapchain
