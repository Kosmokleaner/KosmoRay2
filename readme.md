# KosmoRay2

DXR experiments

by Martin Mittring    1/21/2024

## Ambient Occlusion
<img width="411" alt="image" src="https://github.com/Kosmokleaner/KosmoRay2/assets/44132/98bdd472-3382-44bb-844f-eb731cfb7de7">

## Interpolated Normals visualized

<img width="358" alt="image" src="https://github.com/Kosmokleaner/KosmoRay2/assets/44132/112a28db-9806-4d86-b39f-d11a20158dcc">

## Boolean mesh (triangle mesh against sphere)
https://www.youtube.com/watch?v=XZdRmJa9cKo


## External / Third Party:
* based on LearningDirectX12 DX12Lib https://github.com/jpvanoosten/LearningDirectX12, MIT license
* DearGPU, various helper code, e.g. OBJ loader, WTFPL http://www.wtfpl.net by MartinMittring / Epic Games
  * Tom Forsyth vertex cache optimization code
  * TangentSpaceMeshCalculation by MartinMittring / Crytek
* Cellular noise ("Worley noise") in 3D in GLSL. Stefan Gustavson MIT license. https://github.com/stegu/webgl-noise
* Blue noise textures from https://momentsingraphics.de/Media/BlueNoise/FreeBlueNoiseTextures.zip CC0-licensing
* stb_image to load image file format https://github.com/nothings/stb/blob/master/stb_image.h 