import os

os.system("python waf build -v")
os.system("python -m pip install -U watchdog")
os.system("python ../tools/assetcompiler/AutoCompiler.py")