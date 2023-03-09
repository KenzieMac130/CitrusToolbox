import waflib
import time
import pathlib
import os
import json
import uuid
import shutil

class CitrusAssetCompileTask(waflib.Task.Task):
	name = ""
	globInfo = [] # Required extensions to be responsible for this asset
	paths = [] # Required paths to be responsible for this asset
	isSimpleCopy = False # Only responsible for a simple copy operation
	executableEnv = 'GLTF2Citrus' # Self.env path for the task
	
	# Declares a list of outputs that the task will generate for a given input
	def get_outputs(self, relativePath):
		return ["OUTPUT"] # todo: replace me
		
	# Get a list of dependencies which would trigger a recompile
	def get_dependencies(self, relativePath):
		return []

	def get_asset_type(self, relativePath):
		return None
		
	# Declares a method of filtering files
	def filter(path):
		return True
		
	# Get/set data from context
	def hook_context(self, ctx):
		pass
		
	# Logic to run post execution (cleanup temporary files, renames, etc)
	def hook_postrun(self):
		pass
		
	def load_settings(self, file, declarations):
		#try:
			settings = {}
			settings["args"] = {}
			settings["guids"] = {}
			if not file.exists():
				file.write_json(settings)
			else:
				settings = file.read_json()
			needsRedump = False
			for decl in declarations:
				if decl not in settings["guids"].keys():
					needsRedump = True
					settings["guids"][decl] = uuid.uuid4().hex
			if "type" not in settings:
				assetType = self.get_asset_type(file.relpath())
				if assetType:
					settings["type"] = assetType
					needsRedump = True
			if needsRedump:
				file.write_json(settings)
			self.args.update(self.defaultArgs)
			self.args.update(settings["args"])
			self.guidMapping = settings["guids"]
		#except:
		#	print("FAILED TO LOAD SETTINGS FROM FILE")
		
	# Get a format for a command
	# {EXEC} executable
	# {INPUT[]} input by number
	# {OUTPUT[]} output by number
	# {OUT_GUID[]} output guid by number
	# {...} anything else is user args
	def get_command(self):
		return "{EXEC} -help"
		
	def resolve_file_names(self, declarations):
		results = []
		for decl in declarations:
			results.append(self.guidMapping[decl])
		return results
		
	defaultArgs = {} # Default argument dictionary
	
	args = {}
	guidMapping = {}
	
# ---------------------------------- INTERNAL ---------------------------------- #
	def run(self):
		if self.isSimpleCopy:
			print("Copying " + self.inputs[0].abspath() + " to " + self.outputs[0].abspath())
			shutil.copyfile(self.inputs[0].abspath(), self.outputs[0].abspath())
			return 0
		commandFormat = self.get_command()
		formatDict = {}
		formatDict.update(self.args)
		formatDict["EXEC"] = self.env[self.executableEnv][0]
		print(self)
		formatDict["GUID_SCRIPT"] = pathlib.Path(self.get_cwd().abspath()) / "GUIDLookup.py"
		inputAbsPaths = []
		outputAbsPaths = []
		outputGuids = []
		for input in self.inputs:
			inputAbsPaths.append(input.abspath())
		for output in self.outputs:
			outputAbsPaths.append(output.abspath())
		for guid in self.guidMapping.items():
			outputGuids.append(guid)
		formatDict["INPUT"] = inputAbsPaths
		formatDict["OUTPUT"] = outputAbsPaths
		formatDict["OUTPUT_GUID"] = outputGuids
		commandFormat = commandFormat.format(**formatDict)
		print(commandFormat)
		execRes = self.exec_command(commandFormat)
		self.hook_postrun()
		return execRes

	def scan(self): #Find all output files
		return (self.inputs, time.time())

	def runnable_status(self):
		ret = super(CitrusAssetCompileTask, self).runnable_status()
		bld = self.generator.bld
		return ret