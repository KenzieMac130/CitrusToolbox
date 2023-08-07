import bpy
import numpy as np

bl_info = {
    "name": "Citrus Toolbox Utilities",
    "category": "Generic",
    "version": (1, 0, 0),
    "blender": (3, 4, 0),
    'description': 'Adds various features to create assets for Citrus Toolbox.',
}

def register():
    bpy.utils.register_class(CitrusGltfExtensionProperties)
    bpy.types.Scene.CitrusGltfExtensionProperties = bpy.props.PointerProperty(type=CitrusGltfExtensionProperties)

def unregister():
    unregister_panel()
    bpy.utils.unregister_class(CitrusGltfExtensionProperties)
    del bpy.types.Scene.CitrusGltfExtensionProperties

# ---------------------------------- GLTF STUFF ----------------------------------
glTF_extension_name_spawner = "CITRUS_node_spawner"
glTF_extension_name_constraints = "CITRUS_node_constraints"

class CitrusGltfExtensionProperties(bpy.types.PropertyGroup):
    enabled: bpy.props.BoolProperty(
        name=bl_info["name"],
        description='Include CITRUS extensions in the exported glTF file.',
        default=True
        )
    
    export_splines: bpy.props.BoolProperty(
        name="Export Curves",
        description='Export tagged curves as Citrus splines.',
        default=True
        )
    
    export_spawner_props: bpy.props.BoolProperty(
        name="Export Spawner Props",
        description='Export spawner properties.',
        default=True
        )
    
    export_skeleton_constrains: bpy.props.BoolProperty(
        name="Export Skeleton Constraints",
        description='Export tagged bone constraints as skeleton constraints.',
        default=True
        )

def register_panel():
    # Register the panel on demand, we need to be sure to only register it once
    # This is necessary because the panel is a child of the extensions panel,
    # which may not be registered when we try to register this extension
    try:
        bpy.utils.register_class(GLTF_PT_UserExtensionPanel)
    except Exception:
        pass

    # If the glTF exporter is disabled, we need to unregister the extension panel
    # Just return a function to the exporter so it can unregister the panel
    return unregister_panel

def unregister_panel():
    # Since panel is registered on demand, it is possible it is not registered
    try:
        bpy.utils.unregister_class(GLTF_PT_UserExtensionPanel)
    except Exception:
        pass

if __name__ == "__main__":
    register()

class GLTF_PT_UserExtensionPanel(bpy.types.Panel):

    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Enabled"
    bl_parent_id = "GLTF_PT_export_user_extensions"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator
        return operator.bl_idname == "EXPORT_SCENE_OT_gltf"

    def draw_header(self, context):
        props = bpy.context.scene.CitrusGltfExtensionProperties
        self.layout.prop(props, 'enabled')

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        props = bpy.context.scene.CitrusGltfExtensionProperties
        layout.active = props.enabled

        # todo: UI
        layout.prop(props, 'export_splines', text="Splines")
        layout.prop(props, 'export_spawner_props', text="Spawner Props")
        layout.prop(props, 'export_skeleton_constrains', text="Constraints")

class glTF2ExportUserExtension:

    def __init__(self):
        # We need to wait until we create the gltf2UserExtension to import the gltf2 modules
        # Otherwise, it may fail because the gltf2 may not be loaded yet
        from io_scene_gltf2.io.com.gltf2_io_extensions import Extension
        self.Extension = Extension
        self.properties = bpy.context.scene.CitrusGltfExtensionProperties
        from . import gltf_exts
        self.gltf_exts = gltf_exts

    def gather_joint_hook(self, gltf2_node, blender_bone : bpy.types.Bone, export_settings):
        # todo: bone constraints (bone level)
        pass

    def gather_node_hook(self, gltf2_node, blender_object : bpy.types.Object , export_settings):
        if self.properties.export_splines:
            self.gltf_exts.spline.make_ext(self.Extension, gltf2_node, blender_object)
        # todo: spawner export
        # todo: bone constraints (object level)
        pass
    
    