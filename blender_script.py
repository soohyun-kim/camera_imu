import bpy, mathutils, math, csv

for i in range(100):
    bpy.context.scene.frame_set(i)
    bpy.context.scene.camera.rotation_euler = mathutils.Euler((0.1*i, 0, 0), 'XYZ')
    bpy.context.scene.camera.keyframe_insert(data_path='rotation_euler')
