from .TaskBase import TaskBase
from pathlib import Path
import uuid
import tempfile
import os

class TextureTask(TaskBase):
	name = "texture"

	def poll_file_path(path : Path) -> bool:
		return path.suffix in ['.png', '.jpg', '.jpeg']
		
	def run(self):
		tmp_output = str(Path(tempfile.gettempdir()) / f"{uuid.uuid4().hex}.ktx")
		# todo: options!
		self.exec_command(f"{self.env['Compressonator'][0]} {self.inputs[0].abspath()} {tmp_output}")
		if os.path.exists(self.outputs[0].abspath()):
			os.remove(self.outputs[0].abspath())
		os.rename(tmp_output, self.outputs[0].abspath())
		return 0