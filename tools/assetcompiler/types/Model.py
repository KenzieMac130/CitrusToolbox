from assetcompiler.types.Base import CitrusAssetCompileTask

class Task(CitrusAssetCompileTask):
	name = "Model"
	globInfo = ['**/*.gltf']
	executableEnv = 'CitrusModel' # Self.env path for the task
		
	def get_asset_type(self, relativePath):
		return "model"

	def get_command(self):
		return "{EXEC} -gpu \"{INPUT[0]}\" \"{OUTPUT[0]}\" \"{GUID_SCRIPT}\""