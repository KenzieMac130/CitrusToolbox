from assetcompiler.types.Base import CitrusAssetCompileTask
import os

class Task(CitrusAssetCompileTask):
	name = "Texture"
	globInfo = ['**/*.png', '**/*.jpg', '**/*.jpeg']
	executableEnv = 'Compressonator' # Self.env path for the task
	defaultArgs = {
		"mipmap_count": 8,
		"use_mipmaps": True,
		"format":"BC1"
	}

	def get_asset_type(self, relativePath):
		return "texture"

	def get_command(self):
		print("HEKO" + self.globInfo)
		miplevels = f" -miplevels {self.args['mipmap_count']}"
		if not self.args['use_mipmaps']:
			miplevels = ""
		settings_text = f"-fd {self.args['format']}{miplevels}"
		return f"{{EXEC}} {settings_text} \"{{INPUT[0]}}\" \"{{OUTPUT[0]}}.ktx\""
		
	def hook_postrun(self):
		if os.path.exists(self.outputs[0].abspath()):
			os.remove(self.outputs[0].abspath())
		os.rename(self.outputs[0].abspath()+'.ktx', self.outputs[0].abspath())