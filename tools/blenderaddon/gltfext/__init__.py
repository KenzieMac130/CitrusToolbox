import bpy

class CitrusExtensionProperties(bpy.types.PropertyGroup):
    enabled: bpy.props.BoolProperty(
        name="Citrus Toolbox",
        description='Export Citrus Toolbox Extension.',
        default=False
        )
    curves_enabled: bpy.props.BoolProperty(
        name="Curves Enabled",
        description='Export Citrus Curves.',
        default=False
        )
    spawners_enabled: bpy.props.BoolProperty(
        name="Spawners Enabled",
        description='Export Citrus Spawners.',
        default=False
        )

class GLTF_PT_CitrusExtensionPanel(bpy.types.Panel):
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
        props = bpy.context.scene.CitrusExtensionProperties
        self.layout.prop(props, 'enabled')

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False

        props = bpy.context.scene.CitrusExtensionProperties
        layout.active = props.enabled

        layout.prop(props, 'curves_enabled', "Curves")
        layout.prop(props, 'spawners_enabled', "Spawners")

glTF_extension_curve = "CITRUS_curves"
glTF_extension_spawner = "CITRUS_node_spawner"

class glTF2ExportUserExtension:
    def __init__(self):
        # We need to wait until we create the gltf2UserExtension to import the gltf2 modules
        # Otherwise, it may fail because the gltf2 may not be loaded yet
        from io_scene_gltf2.io.com.gltf2_io_extensions import Extension
        self.Extension = Extension
        self.properties = bpy.context.scene.ExampleExtensionProperties

    def gather_node_hook(self, gltf2_object, blender_object :bpy.types.Object, export_settings):
        # Curves
        if self.properties.curves_enabled:
            if gltf2_object.extensions is None:
                gltf2_object.extensions = {}

            if blender_object.type == 'CURVE' and blender_object.name.startswith("CSP_") :
                data : bpy.types.Curve = blender_object.data
                points = []
                if data.splines:
                    spline = data.splines[0] # only support one spline per object
                    for point in spline.bezier_points: # only bezier is supported right now
                        points.append(point.handle_left)
                        points.append(point.co)
                        points.append(point.handle_right)

                
                gltf2_object.extensions[glTF_extension_curve] = self.Extension(
                    name=glTF_extension_curve,
                    extension={
                        "points": points
                    }
                )

        # Spawner    
        if self.properties.spawners_enabled:
            if gltf2_object.extensions is None:
                gltf2_object.extensions = {}

            spawnProps = {}
            for k, v in blender_object.items():
                if k.startswith("CT_"):
                    spawnProps[k] = v

            gltf2_object.extensions[glTF_extension_spawner] = self.Extension(
                    name=glTF_extension_spawner,
                    extension={
                        "props": points
                    }
                )
