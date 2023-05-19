# Irradiance Cache example

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
