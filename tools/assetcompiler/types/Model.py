from assetcompiler.types.Base import CitrusAssetCompileTask
import shutil

class Task(CitrusAssetCompileTask):
	name = "Model"
	globInfo = ['**/*.gltf']
	executableEnv = 'CitrusModel' # Self.env path for the task
		
	def get_asset_type(self, relativePath):
		return "model"

	def run(self):
		print("DUMMY MODEL PROCESSING! PLEASE REWRITE ASSET COMPILER")
		shutil.copyfile(self.inputs[0].abspath(), self.outputs[0].abspath())
		return 0