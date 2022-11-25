# Pipes the guid for a path and an output to stdout
import sys
import json

# For using as a library
def assetGuidLookup(path,output):
	configFile = open(path+".ctac")
	configData = json.load(configFile)
	configFile.close()
	return configData["guids"][output]

# For using as a command line utility
if __name__ == "__main__":
	try:
		print(assetGuidLookup(sys.argv[1],sys.argv[2]))
	except:
		print("ERROR")
		sys.exit(-1)