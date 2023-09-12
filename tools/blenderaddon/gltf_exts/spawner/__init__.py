import bpy
import idprop

glTF_extension_name_spawner = "CITRUS_node_spawner"

def get_spawner_props_for_object(object : bpy.types.ID) -> dict:
    if object is None:
        return {}
    result = {}
    for key, value in object.items():
        if key.startswith('ct_'):
            name = key.split("ct_")[1]
            if type(value) == idprop.types.IDPropertyArray:
                result[name] = value.to_list()
            else:
                result[name] = value
    return result

def make_ext(extclass, gltf2_node, blender_object : bpy.types.Object):
    spawner_properties = {}
    if blender_object.instance_type == 'COLLECTION':
        spawner_properties.update(get_spawner_props_for_object(blender_object.instance_collection))
    spawner_properties.update(get_spawner_props_for_object(blender_object))

    # do not create extention for empty objects
    if not spawner_properties:
        return

    if gltf2_node.extensions is None:
        gltf2_node.extensions = {}
    gltf2_node.extensions[glTF_extension_name_spawner] = extclass(
        name=glTF_extension_name_spawner,
        required=False,
        extension=spawner_properties
    )