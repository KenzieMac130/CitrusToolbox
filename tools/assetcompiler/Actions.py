import sys
import os
import shutil
import re
import json
from pathlib import Path
import uuid
import subprocess
from .AssetMetadata import AssetMetaData

def run_build():
	subprocess.run(["python", "waf", "build"])

def new_asset(path, asset_template):
	ext = Path(asset_template).suffix
	src_path = Path("engine/asset_templates") / asset_template
	dst_path = Path(path).with_suffix(ext)
	if os.path.exists(dst_path):
		raise RuntimeError("File already exists")
	shutil.copy(src_path, dst_path)
	run_build()


def new_folder(path):
	if not os.path.exists(path):
		os.makedirs(path)

def move(src_path, dst_path):
	if not os.path.exists(dst_path):
		raise RuntimeError("Path does not exist")
	if os.path.isfile(src_path):
		src_ctac = src_path + '.ctac'
		try: 
			shutil.move(src_ctac, dst_path)
		except FileNotFoundError:
			pass
	shutil.move(src_path, dst_path)

def delete(path, build_path_root):
	from send2trash import send2trash
	send2trash(path)
	src_ctac = path + '.ctac'
	try: 
		ctac = open(src_ctac)
		meta = AssetMetaData(ctac)
		ctac.close()
		send2trash(src_ctac)
		for guid in meta.guids.values():
			out_path = Path(build_path_root) / guid.hex
			send2trash(str(out_path))
	except FileNotFoundError:
		pass

def duplicate(path):
	if not os.path.isfile(path):
		raise RuntimeError("Can only duplicate files")
	# get a new path name
	original_path = Path(path)
	original_name = original_path.stem
	new_path = path
	next_number = 1
	while os.path.exists(new_path):
		base_name = original_name
		ext_end_search = re.findall(r"(.*)_(\d\d*$)", Path(new_path).stem)
		try:
			next_number = int(ext_end_search[0][1]) + 1
			base_name = ext_end_search[0][0]
		except IndexError:
			pass
		new_path = original_path.parent / f"{base_name}_{next_number}{original_path.suffix}"
	run_build()

	# copy source file
	shutil.copy(path, new_path)

	# create a duplicate ctac if it exists
	old_ctac_path = f"{path}.ctac"
	new_ctac_path = f"{new_path}.ctac"
	if os.path.exists(old_ctac_path):
		try:
			old_ctac = open(old_ctac_path)
			ctac_data : dict = json.load(old_ctac)
			# get rid of nickname to prevent conflict
			ctac_data.pop("nickname")
			# create new GUIDs
			new_guids = {}
			for key in ctac_data["guids"].keys():
				new_guids[key] = uuid.uuid4().hex
			ctac_data["guids"] = new_guids
			new_ctac = open(new_ctac_path, 'w')
			new_ctac.write(json.dumps(ctac_data, indent=2))
		except KeyError:
			pass

def rename(path, new_name):
	def safe_rename(path, new_name):
		if any(unsafe in new_name for unsafe in ['/', '..', '\\', '.']):
			raise RuntimeError("Unsafe rename provided, remove slashes and dots")
		original_path = Path(path)
		root_path = original_path.absolute()
		extlist = root_path.suffixes
		extension = ''
		for ext in extlist:
			extension = extension + ext
		dst_path = root_path.parent / (new_name + extension)
		os.rename(root_path, dst_path)

	if os.path.isfile(path):
		src_ctac = path + '.ctac'
		try: 
			safe_rename(src_ctac, new_name)
		except FileNotFoundError:
			pass
	safe_rename(path, new_name)