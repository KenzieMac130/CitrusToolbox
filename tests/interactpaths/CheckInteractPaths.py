import re

file = open("InteractPaths.txt")
contents = file.read()
finds = re.findall(r'(\d*)[:]\s*([\S]*)', contents)

pathMap = {}
pathList = []

for line in finds:
    pathList.append(line[1])
    pathMap[line[0]] = line[1]

pathList.sort()

print(pathMap)
print(pathList)

outFile = open("SortedPaths.txt", 'wt')
for line in pathList:
    outFile.write(str(line))
    outFile.write('\n')

outFile.close()
file.close()