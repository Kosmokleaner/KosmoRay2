# KosmoRay2

DirectX 12 Ray tracing experiments

by Martin Mittring
## History:
* Non triangle procedural geometry (analytical spheres in AABB BLAS)
<img width="318" alt="image" src="https://github.com/user-attachments/assets/76500d2e-3d01-4bf6-a586-971208ad57a3">
* 8/22/2024 Beginnings of a Path Tracer (Skylight + AreaLight)
<img width="589" alt="image" src="https://github.com/Kosmokleaner/KosmoRay2/assets/44132/50cf3ac3-76bf-49fe-b260-e75b4c415794">
* Ambient Occlusion
<img width="411" alt="image" src="https://github.com/Kosmokleaner/KosmoRay2/assets/44132/98bdd472-3382-44bb-844f-eb731cfb7de7">
* Interpolated Normals visualized
<img width="358" alt="image" src="https://github.com/Kosmokleaner/KosmoRay2/assets/44132/112a28db-9806-4d86-b39f-d11a20158dcc">
* Boolean mesh (triangle mesh against sphere)
https://www.youtube.com/watch?v=XZdRmJa9cKo


## External / Third Party:
* based on LearningDirectX12 DX12Lib https://github.com/jpvanoosten/LearningDirectX12, MIT license
* DearGPU, various helper code, e.g. OBJ loader, WTFPL http://www.wtfpl.net by MartinMittring / Epic Games
  * Tom Forsyth vertex cache optimization code
  * TangentSpaceMeshCalculation by MartinMittring / Crytek
* Cellular noise ("Worley noise") in 3D in GLSL. Stefan Gustavson MIT license. https://github.com/stegu/webgl-noise
* Blue noise textures from https://momentsingraphics.de/Media/BlueNoise/FreeBlueNoiseTextures.zip CC0-licensing
* stb_image to load image file format https://github.com/nothings/stb/blob/master/stb_image.h 
* DirectXTK from https://github.com/microsoft/DirectXTK12/wiki/ResourceUploadBatch MIT license
* GLM OpenGL math library https://github.com/g-truc/glm
