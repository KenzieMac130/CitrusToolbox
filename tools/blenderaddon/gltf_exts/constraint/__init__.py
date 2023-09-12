import bpy
import numpy as np
import re

glTF_extension_name_constraint = "CITRUS_node_constraint"

# todo: investigate correctness of axis conversion on bones

def get_friction_setting(constraint : bpy.types.Constraint):
    data = re.search(r'F(\d\d\d)', constraint.name)
    if not data:
        return 0.0
    return float(data[1]) / 100.0

def get_ext_for_axis(constraint : bpy.types.LimitRotationConstraint) -> dict:
    return {
        'type':'rotation_axis',
        'friction': get_friction_setting(constraint),
        'limit_x':constraint.use_limit_x,
        'limit_y':constraint.use_limit_z,
        'limit_z':constraint.use_limit_y,
        'min_x':constraint.min_x,
        'max_x':constraint.max_x,
        'min_y':constraint.min_z,
        'max_y':constraint.max_z,
        'min_z':-constraint.max_y,
        'max_z':-constraint.min_y
    }

def get_ext_for_hinge(constraint : bpy.types.LimitRotationConstraint) -> dict:
    min_angle = 0
    max_angle = 0
    axis = 'x'

    if constraint.use_limit_x:
        axis = 'x'
        min_angle = constraint.min_x
        max_angle = constraint.max_x
    elif constraint.use_limit_y:
        axis = 'z'
        min_angle = constraint.min_y
        max_angle = constraint.max_y
    elif constraint.use_limit_y:
        axis = 'y'
        min_angle = -constraint.max_z
        max_angle = -constraint.min_z

    return {
        'type':'rotation_hinge',
        'friction': get_friction_setting(constraint),
        'axis':axis,
        'min_angle':min_angle,
        'max_angle':max_angle,
    }

def get_ext_for_socket(constraint : bpy.types.LimitRotationConstraint) -> dict:
    max_angle = constraint.max_x
    limit_twist = constraint.use_limit_y
    min_twist = constraint.min_y
    max_twist = constraint.max_y
    return {
        'type':'rotation_socket',
        'friction': get_friction_setting(constraint),
        'max_angle':max_angle,
        'limit_twist':limit_twist,
        'min_twist':min_twist,
        'max_twist':max_twist
    }

def process_constraint(extclass, gltf2_node, blender_constraint : bpy.types.Constraint) -> bool:
    name = blender_constraint.name
    mode = ''
    is_exported = False
    if name.endswith('_CXA'):
        is_exported = True
        mode = 'axis'
    if name.endswith('_CXH'):
        is_exported = True
        mode = 'hinge'
    if name.endswith('_CXS'):
        is_exported = True
        mode = 'socket'
    if not is_exported:
        return False # not a citrus constraint, keep looking
    
    if blender_constraint.type != 'LIMIT_ROTATION':
        return False # only support limit constraints for now
    
    if mode == 'socket':
        ext_dict = get_ext_for_socket(blender_constraint)
    elif mode == 'hinge':
        ext_dict = get_ext_for_hinge(blender_constraint)
    else:
        ext_dict = get_ext_for_axis(blender_constraint)

    gltf2_node.extensions[glTF_extension_name_constraint] = extclass(
        name=glTF_extension_name_constraint,
        required=False,
        extension=ext_dict
    )

def make_ext_bone(extclass, gltf2_node, blender_bone : bpy.types.Bone):
    # get ready because this is hacky and potentially slow on large scenes!
    # blender lets you specify multiple armatures with the same data
    # however the constraint data is defined on the object pose, not armature data
    # the gltf exporter only passes an isolated bone from armature data and not the object
    # so we need a way to find the first object with a pose bone set that has a matching bone
    # once we have that we can treat this as our source of data to retrieve constraints
    # note that this rules out instanced armatures but in practice they should not be used
    # so lets just loop through every object looking for a pose bone with our bone... yup
    for obj in bpy.data.objects:
        if obj.pose:
            for pose_bone in obj.pose.bones:
                if pose_bone.bone == blender_bone:
                    for constraint in pose_bone.constraints:
                        if process_constraint(extclass, gltf2_node, constraint):
                            return

def make_ext_object(extclass, gltf2_node, blender_object : bpy.types.Object):
    for constraint in blender_object.constraints:
        if process_constraint(extclass, gltf2_node, constraint):
            return