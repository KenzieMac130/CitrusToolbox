import bpy

from . import gltfext

bl_info = {
    "name": "Citrus Toolbox Utilities",
    "category": "Generic",
    "version": (1, 0, 0),
    "blender": (3, 4, 0),
    'description': 'Adds various features to create assets for Citrus Toolbox.',
}

def register():
    bpy.utils.register_class(gltfext.CitrusExtensionProperties)
    bpy.types.Scene.CitrusExtensionProperties = bpy.props.PointerProperty(type=gltfext.CitrusExtensionProperties)

def register_panel():
    # Register the panel on demand, we need to be sure to only register it once
    # This is necessary because the panel is a child of the extensions panel,
    # which may not be registered when we try to register this extension
    try:
        bpy.utils.register_class(gltfext.GLTF_PT_CitrusExtensionPanel)
    except Exception:
        pass

    # If the glTF exporter is disabled, we need to unregister the extension panel
    # Just return a function to the exporter so it can unregister the panel
    return unregister_panel


def unregister_panel():
    # Since panel is registered on demand, it is possible it is not registered
    try:
        bpy.utils.unregister_class(gltfext.GLTF_PT_CitrusExtensionPanel)
    except Exception:
        pass


def unregister():
    unregister_panel()
    bpy.utils.unregister_class(gltfext.CitrusExtensionProperties)
    del bpy.types.Scene.CitrusExtensionProperties

if __name__ == "__main__":
    register()