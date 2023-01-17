from assetcompiler.types.Base import CitrusAssetCompileTask

class Task(CitrusAssetCompileTask):
	name = "Model"
	globInfo = ['**/*.gltf']
	executableEnv = 'CitrusModel' # Self.env path for the task
	
	# Declares a list of outputs that the task will generate for a given input
	def get_outputs(self, relativePath):
		return ["OUTPUT","GPU"]
		
	def get_asset_type(self, relativePath):
		return "model"

	def get_command(self):
		return "{EXEC} -gpu \"{OUTPUT[1]}\" \"{INPUT[0]}\" \"{OUTPUT[0]}\""