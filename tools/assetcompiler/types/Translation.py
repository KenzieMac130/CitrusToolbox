from assetcompiler.types.Base import CitrusAssetCompileTask
import polib

class Task(CitrusAssetCompileTask):
	name = "Translation"
	globInfo = ['**/*.po']
		
	def run(self):
		po_file = polib.pofile(self.inputs[0].read())
		po_file.save_as_mofile(self.outputs[0].abspath())
		return 0