from assetcompiler import TypeManifest

def build(ctx):
	for taskType in TypeManifest.taskList:
		ctx.add_group(taskType.name)
		
	artifacts = {}
	for taskType in TypeManifest.taskList:
		ctx.set_group(taskType.name)
		print("-------------------- "+ taskType.name + " Task --------------------")
		for file in ctx.path.ant_glob(taskType.globInfo):
			relativePath = file.relpath()
			if not taskType.filter(relativePath):
				continue
			print(relativePath+":")
			task = taskType(env=ctx.env)
			task.hook_context(ctx)
			dependencies = task.get_dependencies(relativePath)
			declarations = task.get_outputs(relativePath)
			settingsFile = ctx.path.make_node(file.relpath() + '.ctac')
			task.load_settings(settingsFile, declarations)
			outFileNames = task.resolve_file_names(declarations)
			outFiles = [] 
			for outName in outFileNames: 
				outFiles.append(ctx.path.find_or_declare(outName))
			inFiles = [file, settingsFile]
			for inName in dependencies:
				inFiles.append(ctx.path.find_or_declare(inName))
			for artifact in zip(outFileNames,declarations):
				artifacts[artifact[0]] = {'in':relativePath,'out':artifact[1]}
			task.set_inputs(inFiles)
			task.set_outputs(outFiles)
			ctx.add_to_group(task)
	artifactFile = ctx.path.find_or_declare("__BUILD_ARTIFACTS__")
	artifactFile.write_json(artifacts)