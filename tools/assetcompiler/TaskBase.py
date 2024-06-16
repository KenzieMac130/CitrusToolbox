import waflib
import time
from pathlib import Path

class TaskBase(waflib.Task.Task):
	# Name of the task
	name = ""

	# Check if task is responsible for file path (before checkin)
	def poll_file_path(path : Path) -> bool:
		return False
	
	# Load metadata for this resource
	def setup(self, ctx, ctac_node, metadata):
		# Metadata
		self.metadata = metadata

		# Input
		path = Path(ctac_node.relpath()).with_suffix('')
		self.set_inputs(ctx.path.find_resource(str(path)))

		# Outputs
		outputs = []
		for guid in self.metadata.guids.values():
			outputs.append(ctx.path.find_or_declare(guid.hex))
		self.set_outputs(outputs)

	def run(self): # Logic that runs the command
		return 0
	
	def scan(self): # Find all input files
		return (self.inputs, time.time())

	def runnable_status(self): # Gets the runnable status
		return super(TaskBase, self).runnable_status()