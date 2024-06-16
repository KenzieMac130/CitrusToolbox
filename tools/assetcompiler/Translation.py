from .TaskBase import TaskBase
from pathlib import Path
import polib

class TranslationTask(TaskBase):
	name = "translation"

	def poll_file_path(path : Path) -> bool:
		return path.suffix in ['.po']
		
	def run(self):
		po_file = polib.pofile(self.inputs[0].read())
		po_file.save_as_mofile(self.outputs[0].abspath())
		return 0