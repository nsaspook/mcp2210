import bpy
from mathutils import Quaternion

filename = 'input.txt'

file = open(filename, 'r')
lines = file.readlines()

frame_number = 0

ob = bpy.context.object 
ob.rotation_mode = 'QUATERNION'

for i in lines:
    frame_number += 1
    q0, q1, q2, q3 = i.split(',')
    quat = float(q0), float(q1), float(q2), float(q3)

    ob.rotation_quaternion = quat
    ob.keyframe_insert('rotation_quaternion', frame=frame_number)

file.close()
