required = {'glob2'}
import os,platform
import shutil
from pathlib import Path;
pid = os.getpid()

cwd = Path(__file__).parent.absolute()
os.chdir(cwd)
output_dir = "out"
full_path_output_dir = cwd.joinpath(output_dir)

import sysconfig
import pyjit
import numpy
python_info = sysconfig.get_paths()
Python3_INCLUDE_DIRS = python_info["include"]
numpy_path = Path(numpy.__file__)
Python3_NumPy_INCLUDE_DIRS = numpy_path.parent.absolute().joinpath('core','include')

path = Path(pyjit.__file__)
pyjit_includePath = path.parent.absolute()

yaml_includePath = "external/yaml-cpp/include"

includes =[
    Python3_INCLUDE_DIRS,
    Python3_NumPy_INCLUDE_DIRS,
    pyjit_includePath,
    yaml_includePath,
    "src/Include",
    "src/Common",
    "src/Network",
    "src/DataView",
    "src/DataFrame",
    "src/Core",
    "src/Log",
    "src/Main",
    "src/Database",
    "src/Tasks",
    "src/Streaming"
    ]

SRC_Common =["src/Common/*.cpp"]
SRC_DataView =["src/DataView/*.cpp"]
SRC_DataFrame =["src/DataFrame/frame.cpp", 
      "src/DataFrame/tensor.cpp",
      "src/DataFrame/FramePack.cpp",
      "src/DataFrame/supernode.cpp"]
SRC_Network =["src/Network/*.cpp"]
SRC_Log =["src/Log/*.cpp"]
SRC_Core =["src/Core/*.cpp"]
SRC_Main =["src/Main/*.cpp"]
SRC_Database =["src/Database/*.cpp","src/Database/sqlite/sqlite3.c"]
SRC_Tasks = ["src/Tasks/*.cpp"]
SRC_Streaming =["src/Streaming/*.cpp"]

YAML_SRC =["external/yaml-cpp/src/*.cpp"]


macros = [("YAML_CPP_STATIC_DEFINE",None)]
if(platform.system() =="Windows"):
    macros+=[("WIN32",1)]

libraries =[]
if(platform.system() =="Windows"):
    libraries+=["Ws2_32","Ole32"]
else:
    libraries+=["pthread","dl","uuid"]

print("Start building Cantor...")
pyjit.build(SRC_Common,
            SRC_DataView,
            SRC_DataFrame,
            SRC_Network,
            SRC_Log,
            SRC_Core,
            SRC_Main,
            SRC_Database,
            SRC_Tasks,
            YAML_SRC,
            SRC_Streaming,
            macros=macros,
            #extra_preargs=["/std:c++17"],
            target_name="cantor",
            target_type = "exe",
            target_lang = "c++",
            libraries =libraries,
            debug=True,
            include_dirs = includes,
            output_dir=output_dir)

os.chdir(cwd)
shutil.copytree("Config",output_dir+"/Config",dirs_exist_ok=True,
    ignore = shutil.ignore_patterns("__pycache__"))
shutil.copytree("Script",output_dir+"/Script",dirs_exist_ok=True,
    ignore = shutil.ignore_patterns("__pycache__"))

print("End to build Cantor.")
