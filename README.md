# CoD4-dm1
---
CoD4 Cameara Tool allows to dump camera information from the game on runtime, which can be imported to other 3D Software (Blender, UnrealEngine, Maya)

This repository designed to be used with [C2M](https://github.com/sheilan102/C2M/tree/master) so camera dump and map can be merged.

### This Alpha Release Includes:

- Simple ingame indicator
- CSV format for export
- Python script for importing csv in to Blender
- Directx 9 Hook


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

Delete to unhook from the game.


### Building:
Requirements
1. [Visual Studio](https://visualstudio.microsoft.com/)
2. [DirectX SDK](https://www.microsoft.com/en-pk/download/details.aspx?id=6812)




```
git clone https://github.com/P-pisiko/CoD4_camera_Tool
```
Open solution -> Release Mode -> x86 -> Build