from .TaskBase import TaskBase
from pathlib import Path

class ModelTask(TaskBase):
	name = "model"

	def poll_file_path(path : Path) -> bool:
		return path.suffix in ['.gltf']
		
	def run(self):
		return self.exec_command(f"{self.env['CitrusModel'][0]} {self.inputs[0].abspath()} {self.outputs[0].abspath()}")