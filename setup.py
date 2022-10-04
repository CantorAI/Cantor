import os,platform,glob,sysconfig
from pathlib import Path
import pyjit
from setuptools import setup,Extension

#from tkinter import messagebox
#messagebox.showinfo(title="Cantor Setup",message=os.getpid())

cwd = Path(__file__).parent.absolute()
os.chdir(cwd)

ver = '1.5.1701'
app_name='cantor'

python_info = sysconfig.get_paths()
Python3_INCLUDE_DIRS = python_info["include"]

path = Path(pyjit.__file__)
pyjit_includePath = path.parent.absolute()

yaml_includePath = "external/yaml-cpp/include"

extern_includes =[
    Python3_INCLUDE_DIRS,
    pyjit_includePath,
    yaml_includePath,
    ]
yaml_src = glob.glob('external/yaml-cpp/src/*.cpp')
src_cpp = glob.glob('src/*/*.cpp')
sqlite_c = glob.glob('src/*/sqlite/sqlite3.c')
src = yaml_src + src_cpp + sqlite_c

PYJIT_SRC =["Python/cantor/Frame.py"]
Stubs_Abs = pyjit.generate(PYJIT_SRC,out="build/gen_out")
Stubs =[]
for stub in Stubs_Abs:
    stub = os.path.relpath(stub, cwd)
    Stubs.append(stub)
src+=Stubs

libs =[]
macros = [("BIND_TO_CANTOR",None)]
macros += [("COMPILE_AS_SHAREDLIB",None)]
macros += [("YAML_CPP_STATIC_DEFINE",None)]


if platform.system() == 'Windows':
    macros += [('WIN32', '1')]
    libs +=['Ws2_32','user32','Ole32']
else:
    libs +=['pthread','dl','uuid']

src_inc =[
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
includes = extern_includes + src_inc

#use this format:'cantor.russell' to put extension into cantor folder
cantor_engine_module = Extension('cantor.russell',
                    sources = src,
                    include_dirs=includes,
                    libraries =libs,
                    define_macros=macros)

long_description ='''
    Distributed Computing System for AI and More;
    Cluster Computing,Remoting,Real time streaming computing
'''

setup(
    name="cantor",
    version=ver,
    author="Galaxy3DVision Team",
    author_email="Olivia@galaxy3dvision.com",
    description="Distributed Computing System for AI and More",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="http://www.galaxy3dvision.com",
    project_urls=
    {
        "Bug Tracker": "http://www.galaxy3dvision.com/pyjitproject/issues",
    },
    keywords=(
        "distributed-computing"
        "cantor pyjit deep-leraning AI ai machine-learning"
        "pathon remoting streaming real-time"
        ),
    classifiers=
    [
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    install_requires=
    [
       "setuptools",
    ],

    entry_points=
    {
        "console_scripts": 
        [
            "cantor=cantor:cli.main",
        ]
    },
    include_package_data=True,
    #for data file, add sub-package use .
    #and the package_data's file pattern path uses the 
    #path inside package_dir
    #for example, cantor.Config,should use *.yaml,
    #don't include Config path
    packages =[
        "cantor",
        "cantor.Config"
        ],
    package_dir={
        "cantor": "Python/cantor",
        "cantor.Config":"Config"
     },
    package_data={
        'cantor.Config': ['*.yaml']},

    python_requires=">=3.8",
    ext_modules = [cantor_engine_module]
)
