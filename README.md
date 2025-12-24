# CoD4-dm1
---
CoD4 Cameara Tool allows to dump camera information from the game on runtime, which can be imported to other 3D Software (Blender, UnrealEngine, Maya)

This repository designed to be used with [C2M](https://github.com/sheilan102/C2M/tree/master) so camera dump and map can be merged.

### This Alpha Release Includes:

- Support for 1.7 CoD4x 21.1 for now (newer version of CoD4x should work fine)
- Simple ingame indicator
- CSV and Gltf format export
- Python script for importing csv in to Blender
- Directx 9 Hook

Video Demo:

[![Watch on Odysee](https://thumbnails.odycdn.com/optimize/s:390:220/quality:85/plain/https://thumbs.odycdn.com/caf0c0b364c9cdbe885a1565054724da.webp&quot)](https://odysee.com/@nakedleisure:f/Cod4CamTool:5)


## Building & Usage

### Usage:
Download the latest [release](https://github.com/P-pisiko/CoD4_camera_Tool/releases)

__Warning:__ MAKE SURE GAME IS RUNNING ON BORDERLESS MODE
```
r_fullscreen 0
vid_restart
# if your resolution is set to native, game window should look exacly the same.
```

Start the __CoD4-dm1.exe__ then launch the game.



Numpad 5 to controll *(start/stop)* the capture.

HOME key to unhook from the game.


### Building:
Requirements
1. [Visual Studio](https://visualstudio.microsoft.com/)
2. [DirectX SDK](https://www.microsoft.com/en-pk/download/details.aspx?id=6812)




```
git clone https://github.com/P-pisiko/CoD4_camera_Tool
```
Open solution -> Release Mode -> x86 -> Build
