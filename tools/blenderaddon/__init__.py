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
    try:
        bpy.utils.register_class(GLTF_PT_UserExtensionPanel)
    except Exception:
        pass
    return unregister_panel

def unregister_panel():
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
        layout.use_property_decorate = False

        props = bpy.context.scene.CitrusGltfExtensionProperties
        layout.active = props.enabled

        layout.prop(props, 'export_splines', text="Splines")
        layout.prop(props, 'export_spawner_props', text="Spawner Props")
        layout.prop(props, 'export_skeleton_constrains', text="Constraints")

class glTF2ExportUserExtension:

    def __init__(self):
        from io_scene_gltf2.io.com.gltf2_io_extensions import Extension
        self.Extension = Extension
        self.properties = bpy.context.scene.CitrusGltfExtensionProperties
        from . import gltf_exts
        self.gltf_exts = gltf_exts

    def gather_joint_hook(self, gltf2_node, blender_bone : bpy.types.Bone, export_settings):
        if self.properties.export_skeleton_constrains:
            self.gltf_exts.constraint.make_ext_object(self.Extension, gltf2_node, blender_bone)

    def gather_node_hook(self, gltf2_node, blender_object : bpy.types.Object , export_settings):
        if self.properties.export_splines:
            self.gltf_exts.spline.make_ext(self.Extension, gltf2_node, blender_object)
        if self.properties.export_spawner_props:
            self.gltf_exts.spawner.make_ext(self.Extension, gltf2_node, blender_object)
        if self.properties.export_skeleton_constrains:
            self.gltf_exts.constraint.make_ext_object(self.Extension, gltf2_node, blender_object)