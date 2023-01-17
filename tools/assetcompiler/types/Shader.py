from assetcompiler.types.Base import CitrusAssetCompileTask

class Task(CitrusAssetCompileTask):
	name = "Shader"
	globInfo = ['**/*.ctsi']
	executableEnv = 'CitrusShader' # Self.env path for the task

	def get_asset_type(self, relativePath):
		return "shader"
		
	def get_command(self):
		return "{EXEC} {INPUT[0]} {OUTPUT[0]} {type}"
		
	defaultArgs = {"type":"-surface"}