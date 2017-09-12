#!/usr/bin/python

import os
import re
import sys
import string
import json
from shutil import copyfile

ID_KEYWORD = "projectid"
NAMESPACE_KEYWORD = "ProjectNamespace"
PRETTYNAME_KEYWORD = "Template"
METATYPE_KEYWORD = "templatetype"
ID_PATTERN = "^([a-z0-9]+)$"
NAMESPACE_PATTERN = "^([A-Za-z][A-Za-z0-9]+)$"
PRETTYNAME_PATTERN = "^([A-Za-z0-9 _\\-]+)$"
TEMPLATE_EXTENSION_BASE = "templateExtension/"
CMAKE_PATTERN = "^add_subdirectory\(([^\)]+)\)$"

if len(sys.argv) != 4:
    u = "Usage: create_plugin.py <id [a-z0-9]> <namespace> <Pretty Name>\n"
    sys.stderr.write(u)
    sys.exit(1)

id_regex = re.compile(ID_PATTERN)
namespace_regex = re.compile(NAMESPACE_PATTERN)
prettyname_regex = re.compile(PRETTYNAME_PATTERN)
cmake_regex = re.compile(CMAKE_PATTERN)

id_string = sys.argv[1]
namespace_string = sys.argv[2]
prettyname_string = sys.argv[3]

if not id_regex.match(id_string):
    e = "ID has to match " + ID_PATTERN + "\n"
    sys.stderr.write(e)
    sys.exit(1)

if not namespace_regex.match(namespace_string):
    e = "Namespace has to match " + NAMESPACE_PATTERN + "\n"
    sys.stderr.write(e)
    sys.exit(1)

if not prettyname_regex.match(prettyname_string):
    e = "Pretty Name has to match " + PRETTYNAME_PATTERN + "\n"
    sys.stderr.write(e)
    sys.exit(1)

raw_input("Are we in the src/plugins directory? If not do not proceed because it won't work! ")

print("Creating directory . . .")
os.mkdir(id_string)
os.chdir(id_string)

baseDir = os.path.join("..", TEMPLATE_EXTENSION_BASE)
filesToPrepare = []

def scanDir(toscan):
    global filesToPrepare
    global baseDir
    if toscan:
        template = os.path.join(baseDir, toscan)
    else:
        template = baseDir
    ext = toscan
    files = os.listdir(template)
    for nextFile in files:
        path = os.path.join(template, nextFile)
        pathNew = os.path.join(ext, nextFile)
        if os.path.isdir(path):
            print("Creating directory " + pathNew)
            os.mkdir(pathNew)
            scanDir(os.path.join(toscan, nextFile))
        else:
            print("Copying file " + pathNew)
            copyfile(path, pathNew)
            filesToPrepare.append(pathNew)

scanDir("")

for localFile in filesToPrepare:
    print("Preparing file " + localFile)
    with open(localFile) as fd:
        tmpfile = localFile + ".tmp"
        tmp = open(tmpfile, "w")
        for line in fd:
            tmp.write(re.sub(ID_KEYWORD, id_string, 
                    re.sub(NAMESPACE_KEYWORD, namespace_string, 
                            re.sub(PRETTYNAME_KEYWORD, prettyname_string, 
                                    re.sub(METATYPE_KEYWORD, "normal", line)))))
        tmp.close()
        os.rename(tmpfile, localFile)

print("Leaving directory . . .")
os.chdir("..")
print("Updating CMakeLists.txt . . . ")
sectionHeadings = {
        "normal":   "Extension plugins",
        "frontend": "Frontend plugins",
        "debug":    "Non-release plugins"
}
existingExtensions = {
        "normal": [],
        "frontend": [],
        "debug": []
}
preLines = '# Do not export symbols by default\nset(CMAKE_CXX_VISIBILITY_PRESET hidden)\nset(CMAKE_VISIBILITY_INLINES_HIDDEN 1)\n\n# Set the RPATH for the library lookup\nset(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/albert")\n\n'
debugIfClause = "# Non-release plugins\nif(${BUILD_DEBUG_EXTENSIONS})\n"
debugEndIfClause = "endif(${BUILD_DEBUG_EXTENSIONS})\n"

# Get all the plugin dirs
pluginDirs = [name for name in os.listdir(".") if os.path.isdir(name)]

plugins = {
        "normal": [],
        "frontend": [],
        "else": []
}
headings = {
        "normal": "# Extension Plugins\n",
        "frontend": "# Frontend Plugins\n"
}

for pDir in pluginDirs:
    with open(os.path.join(pDir, "metadata.json"), "r") as metadataFile:
        metadata = json.load(metadataFile)
        if metadata["type"] != "normal" and metadata["type"] != "frontend":
            plugins["else"].append(pDir)
        else:
            plugins[metadata["type"]].append(pDir)

plugins["normal"].sort()
plugins["frontend"].sort()
plugins["else"].sort()


with open("CMakeLists.txt", "w") as cmakefile:
    cmakefile.write(preLines)
    for section in [ "normal", "frontend" ]:
        cmakefile.write(headings[section])
        for plugin in plugins[section]:
            cmakefile.write("add_subdirectory(")
            cmakefile.write(plugin)
            cmakefile.write(")\n")
        cmakefile.write("\n")
    cmakefile.write(debugIfClause)
    for plugin in plugins["else"]:
        cmakefile.write("    add_subdirectory(")
        cmakefile.write(plugin)
        cmakefile.write(")\n")
    cmakefile.write(debugEndIfClause)


