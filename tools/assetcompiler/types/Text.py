from assetcompiler.types.Base import CitrusAssetCompileTask
import shutil

class Task(CitrusAssetCompileTask):
	name = "Text"
	globInfo = ['**/*.json','**/*.txt','**/*.lua']

	def get_asset_type(self, relativePath):
		if ".lua" in relativePath:
			return "lua"
		else:
			return "text"
	
	def filter(path):
		banList = ["cmake","readme","license","datanicknames","assettypeinfo"]
		if any(ban in path.lower() for ban in banList):
			return False
		return True
	
	def run(self):
		print("Processing Text " + self.inputs[0].abspath() + " to " + self.outputs[0].abspath())
		shutil.copyfile(self.inputs[0].abspath(), self.outputs[0].abspath())
		return 0