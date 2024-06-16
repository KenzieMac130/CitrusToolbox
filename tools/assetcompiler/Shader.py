from .TaskBase import TaskBase
from pathlib import Path

class ShaderTask(TaskBase):
	name = "shader"

	def poll_file_path(path : Path) -> bool:
		return path.suffix in ['.ctsi']
		
	def run(self):
		return self.exec_command(f"{self.env['CitrusShader'][0]} {self.inputs[0].abspath()} {self.outputs[0].abspath()}")