"""
COD4 Blender camera importer
─────────────────────────────
 reads a CSV expor
 
"""

import bpy
import csv
import math
from pathlib import Path

# ────────────────────────────────
#   SETTINGS
# ────────────────────────────────
CSV_PATH      = Path(r"C:\Users\<YOUR-USERNAME>\positions.csv")  # <‑‑ change me
RECORD_FPS    = 125                               # match your capture
INCH_TO_METRE = 0.0254                            # 1 inch = 0.0254 m
CAMERA_NAME   = "Camera"                          
# If your CSV already has Y negated, set FLIP_Y = False
FLIP_Y        = False
# ────────────────────────────────

# ensure we have a camera object
if CAMERA_NAME in bpy.data.objects:
    cam = bpy.data.objects[CAMERA_NAME]
else:
    cam_data = bpy.data.cameras.new(CAMERA_NAME)
    cam = bpy.data.objects.new(CAMERA_NAME, cam_data)
    bpy.context.scene.collection.objects.link(cam)

cam.rotation_mode = 'XYZ'

scene = bpy.context.scene
scene.render.fps = RECORD_FPS     
scene.render.fps_base = 1.0

max_frame = 0

def cod4_pitch_to_blender(pitch_deg: float) -> float:
    """Convert wrapped CoD4 pitch to Blender radians (X axis)."""
    signed = -pitch_deg if pitch_deg <= 85 else 360.0 - pitch_deg
    return math.radians(signed)

def cod4_yaw_to_blender(yaw_deg: float) -> float:
    """Convert CoD4 yaw to Blender radians (Z axis)."""
    return math.radians(-yaw_deg)   # flip because Blender's +Y is back

with CSV_PATH.open(newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        frame_idx = int(row["frame"])
        max_frame = max(max_frame, frame_idx)

        # position
        x = float(row["x"]) * INCH_TO_METRE
        y_raw = float(row["y"])
        y = (-y_raw if FLIP_Y else y_raw) * INCH_TO_METRE
        z = float(row["z"]) * INCH_TO_METRE

        # rotation
        pitch = cod4_pitch_to_blender(float(row["pitch"]))
        yaw   = cod4_yaw_to_blender(float(row["yaw"]))
        roll  = 0.0 

        scene.frame_set(frame_idx)
        cam.location       = (x, y, z)
        cam.rotation_euler = (pitch, 0.0, yaw+180)

        cam.keyframe_insert(data_path="location",       frame=frame_idx)
        cam.keyframe_insert(data_path="rotation_euler", frame=frame_idx)

scene.frame_start = 0
scene.frame_end   = max_frame
print(f"Imported {max_frame + 1} frames at {RECORD_FPS} fps from {CSV_PATH.name}")
