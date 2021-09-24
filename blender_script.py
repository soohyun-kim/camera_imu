import bpy, mathutils, math, csv, os

os.chdir('/camera_imu/results')
if len(os.listdir())==0:
    print("No files!")
for fileinc in range(len(os.listdir())):
    dataReader = csv.reader(open(str(fileinc)+'.csv'))

    camInitRotation = bpy.data.objects.new('camInitRot'+str(fileinc), None)
    bpy.context.scene.collection.objects.link(camInitRotation)
    
    cameraController = bpy.data.objects.new('cameraController'+str(fileinc), None)
    bpy.context.scene.collection.objects.link(cameraController)
    
    cameraController.parent = camInitRotation

    i = 0
    for frame in dataReader:
        angleList = []
        angleList.append(-float(frame[0]))
        angleList.append(0)
        angleList.append(float(frame[2]))
        bpy.context.scene.frame_set(i)
        bpy.data.objects['cameraController'+str(fileinc)].rotation_euler = mathutils.Euler(angleList)
        bpy.data.objects['cameraController'+str(fileinc)].keyframe_insert(data_path='rotation_euler')
        i = i + 1

