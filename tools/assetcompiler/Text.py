from .TaskBase import TaskBase
from pathlib import Path
import shutil

class TextTask(TaskBase):
	name = "text"

	def poll_file_path(path : Path) -> bool:
		return path.suffix in ['.txt', '.json']
		
	def run(self):
		shutil.copyfile(self.inputs[0].abspath(), self.outputs[0].abspath())