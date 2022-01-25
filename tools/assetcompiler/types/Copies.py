from assetcompiler.types.Base import CitrusAssetCompileTask

class Task(CitrusAssetCompileTask):
	name = "Copies"
	globInfo = ['**/*.json','**/*.txt','**/*.lua']
	isSimpleCopy = True # Only responsible for a simple copy operation
	
	def filter(path):
		banList = ["cmake","readme","license","datanicknames","assettypeinfo"]
		if any(ban in path.lower() for ban in banList):
			return False
		return True