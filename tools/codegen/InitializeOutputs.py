import sys
from pathlib import Path

def ensure_path_exists(path : Path):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.touch()

def main():
    codegenPath = Path(sys.argv[1])
    codegenPath = codegenPath / "codegen"
    projectRoot = Path(sys.argv[2])
    inputPaths = open(sys.argv[3]).read().split(';')
    outlist = set()
    for sInputPath in inputPaths:
        inputPath = Path(sInputPath)
        relInputPath = inputPath.relative_to(projectRoot)
        outHeaderPath = codegenPath / relInputPath
        outImplPath = outHeaderPath.with_suffix('.cpp')
        ensure_path_exists(outHeaderPath)
        ensure_path_exists(outImplPath)
        outlist.add(str(outHeaderPath).replace("\\", "/"))
        outlist.add(str(outImplPath).replace("\\", "/"))
    sys.stdout.write(';'.join(outlist))

if __name__ == "__main__":
    main()