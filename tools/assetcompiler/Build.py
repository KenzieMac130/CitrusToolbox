import os
import time
from pathlib import Path
from .AssetMetadata import AssetMetaData
from . import TaskList
import waflib

def query_type_for_asset(path):
	for task_type in TaskList.task_types:
		if task_type.poll_file_path(Path(path)):
			return task_type.name
	return None

def checkin_asset(path):
	ctac_path = Path(f"{path}.ctac")
	if ctac_path.exists():
		return
	atype = query_type_for_asset(path)
	if atype == None:
		return
	metadata = AssetMetaData()
	metadata.type = atype
	ctac_file = ctac_path.open("w")
	metadata.dump(ctac_file)
	ctac_file.close()

def should_process_node(node) -> bool:
	if node.suffix() in [".ctac", ".py", ".pyc", ".bat"]:
		return False
	if node.name in ["CMakeLists.txt", "waf", "wscript", "editoractions"]:
		return False
	if "asset_templates" in node.relpath():
		return False
	return True

def build_step_checkin_all_assets(ctx):
	ctx.add_group("checkin")
	for file in ctx.path.ant_glob():
		if not should_process_node(file):
			continue
		checkin_asset(file.abspath())

def build_step_compile(ctx):
	for task_type in TaskList.task_types:
		ctx.add_group(task_type.name)
		for file in ctx.path.ant_glob("**/*.ctac"):
			metadata = AssetMetaData(dict = file.read_json())
			if metadata.type == task_type.name:
				task = task_type(env=ctx.env)
				task.setup(ctx, file, metadata)
				ctx.add_to_group(task)

mapping_file_guid = 'b48f4ee47fb142aea5c6c8416eff85f4'
def build_step_generate_mapping(ctx):
	print("mapping")
	nickname_dict = {}
	for file in ctx.path.ant_glob("**/*.ctac"):
		metadata = AssetMetaData(dict = file.read_json())
		if metadata.nickname:
			for name, guid in metadata.guids.items():
				postfix = ""
				if name != "OUTPUT":
					postfix = name
				nickname_dict[metadata.nickname + postfix] = guid.hex
	mapping_file = ctx.bldnode.make_node(mapping_file_guid)
	mapping_file.write_json(nickname_dict)