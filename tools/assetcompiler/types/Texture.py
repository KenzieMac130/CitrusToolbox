from assetcompiler.types.Base import CitrusAssetCompileTask
import os

class Task(CitrusAssetCompileTask):
	name = "Texture"
	globInfo = ['**/*.png', '**/*.jpg', '**/*.jpeg']
	executableEnv = 'Compressonator' # Self.env path for the task
		
	def get_command(self):
		return "{EXEC} -miplevels 8 \"{INPUT[0]}\" \"{OUTPUT[0]}.ktx\""
		
	def hook_postrun(self):
		if os.path.exists(self.outputs[0].abspath()):
			os.remove(self.outputs[0].abspath())
		os.rename(self.outputs[0].abspath()+'.ktx', self.outputs[0].abspath())