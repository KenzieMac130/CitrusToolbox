import bpy
import numpy as np
from io_scene_gltf2.io.com import gltf2_io_constants
from io_scene_gltf2.blender.exp import gltf2_blender_gather_primitive_attributes

glTF_extension_name_spline = "CITRUS_node_spline"

def get_curve_export_geonodes():
        # if exists return
        if "CT_CURVE_EXPORT" in bpy.data.node_groups:
            return bpy.data.node_groups["CT_CURVE_EXPORT"]
        
        # otherwise create the node group
        ct_curve_export= bpy.data.node_groups.new(type = 'GeometryNodeTree', name = "CT_CURVE_EXPORT")
        ct_curve_export.inputs.new('NodeSocketGeometry', "Geometry")
        ct_curve_export.inputs[0].attribute_domain = 'POINT'
        group_input = ct_curve_export.nodes.new("NodeGroupInput")
        curve_tangent = ct_curve_export.nodes.new("GeometryNodeInputTangent")
        store_named_attribute_001 = ct_curve_export.nodes.new("GeometryNodeStoreNamedAttribute")
        store_named_attribute_001.data_type = 'FLOAT_VECTOR'
        store_named_attribute_001.domain = 'POINT'
        store_named_attribute_001.inputs[1].default_value = True
        store_named_attribute_001.inputs[2].default_value = "cnormal"
        store_named_attribute_001.inputs[4].default_value = 0.0
        store_named_attribute_001.inputs[5].default_value = (0.0, 0.0, 0.0, 0.0)
        store_named_attribute_001.inputs[6].default_value = False
        store_named_attribute_001.inputs[7].default_value = 0
        store_named_attribute = ct_curve_export.nodes.new("GeometryNodeStoreNamedAttribute")
        store_named_attribute.data_type = 'FLOAT_VECTOR'
        store_named_attribute.domain = 'POINT'
        store_named_attribute.inputs[1].default_value = True
        store_named_attribute.inputs[2].default_value = "ctangent"
        store_named_attribute.inputs[4].default_value = 0.0
        store_named_attribute.inputs[5].default_value = (0.0, 0.0, 0.0, 0.0)
        store_named_attribute.inputs[6].default_value = False
        store_named_attribute.inputs[7].default_value = 0
        normal = ct_curve_export.nodes.new("GeometryNodeInputNormal")
        ct_curve_export.outputs.new('NodeSocketGeometry', "Geometry")
        ct_curve_export.outputs[0].attribute_domain = 'POINT'
        group_output = ct_curve_export.nodes.new("NodeGroupOutput")
        ct_curve_export.links.new(group_input.outputs[0], store_named_attribute.inputs[0])
        ct_curve_export.links.new(curve_tangent.outputs[0], store_named_attribute.inputs[3])
        ct_curve_export.links.new(store_named_attribute.outputs[0], store_named_attribute_001.inputs[0])
        ct_curve_export.links.new(normal.outputs[0], store_named_attribute_001.inputs[3])
        ct_curve_export.links.new(store_named_attribute_001.outputs[0], group_output.inputs[0])
        return ct_curve_export

def sptn_from_curve(blender_object : bpy.types.Object):
    positions = []
    tangents = []
    normals = []
    
    # todo: apply geometry nodes to get tangents/normals
    curve_nodes = get_curve_export_geonodes()
    modifier = blender_object.modifiers.new(name='ApplyCurves', type='NODES')
    modifier.node_group = curve_nodes
    depsgraph = bpy.context.evaluated_depsgraph_get()
    blender_object_eval : bpy.types.Object = blender_object.evaluated_get(depsgraph)

    # todo evaluate as mesh
    mesh : bpy.types.Mesh = blender_object_eval.to_mesh()
    for v in mesh.vertices:
        loc = v.co
        norm = mesh.attributes['cnormal'].data[v.index].vector
        tan = mesh.attributes['ctangent'].data[v.index].vector

        # y is always assumed up
        positions.append(loc.x)
        positions.append(loc.z)
        positions.append(-loc.y)

        normals.append(norm.x)
        normals.append(norm.z)
        normals.append(-norm.y)

        tangents.append(tan.x)
        tangents.append(tan.z)
        tangents.append(-tan.y)

    # clean up
    blender_object.modifiers.remove(modifier)

    return positions, tangents, normals

def make_ext(extclass, gltf2_node, blender_object : bpy.types.Object):
    if blender_object.type != 'CURVE': return
    data : bpy.types.Curve = blender_object.data
    if not data.splines: return
    spline = data.splines[0] # only one interpolation type per spline

    # setup spline interpretation
    spline_type = spline.type
    if(spline_type not in ['POLY', 'BEZIER']): return
    spline_type = 'linear' if spline_type == 'POLY' else 'cubic'
    
    # read out data
    positions, tangents, normals = sptn_from_curve(blender_object)

    # setup extension
    if gltf2_node.extensions is None:
        gltf2_node.extensions = {}
    gltf2_node.extensions[glTF_extension_name_spline] = extclass(
        name=glTF_extension_name_spline,
        extension={
            "type": spline_type,
            "cyclic" : spline.use_cyclic_u,
            "attributes": {
                "POSITION": gltf2_blender_gather_primitive_attributes.array_to_accessor(
                    np.array(positions, dtype=np.float32).reshape(int(len(positions) / 3), 3),
                    component_type=gltf2_io_constants.ComponentType.Float,
                    data_type=gltf2_io_constants.DataType.Vec3,
                    include_max_and_min=True,
                ),
                "NORMAL": gltf2_blender_gather_primitive_attributes.array_to_accessor(
                    np.array(normals, dtype=np.float32).reshape(int(len(normals) / 3), 3),
                    component_type=gltf2_io_constants.ComponentType.Float,
                    data_type=gltf2_io_constants.DataType.Vec3,
                    include_max_and_min=False,
                ),
                "TANGENT": gltf2_blender_gather_primitive_attributes.array_to_accessor(
                    np.array(tangents, dtype=np.float32).reshape(int(len(tangents) / 3), 3),
                    component_type=gltf2_io_constants.ComponentType.Float,
                    data_type=gltf2_io_constants.DataType.Vec3,
                    include_max_and_min=False,
                ),
            }
        }
    )