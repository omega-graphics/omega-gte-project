#
# A simple script for compiling shaders
#

import os
import sys
import argparse
import shutil


parser = argparse.ArgumentParser(description="A python script used for compiling shader files")


parser.add_argument("--metallib",dest="metallib",action='store_const',const=True,default=False)
parser.add_argument("--hlsl",dest="hlsl",action='store_const',const=True,default=False)
parser.add_argument("--shader-type",type=str,help="Defines the shader type to compile...\nNote: this must be one of the following options:\n\nvertex, fragment, or compute")
parser.add_argument("--entry-point",type=str)
parser.add_argument("-O",type=str)
parser.add_argument("files",metavar='files',nargs="+",type=str)
args = parser.parse_args()
if args.files == None:
    print("No input Files.. Exiting..")
    exit(1)
else:
    if args.metallib:
        # Metal Shader Compilation
        air_files = []
        for _f in args.files:
            end_idx = len(_f)-1
            res = os.path.basename(_f)
            res += ".air"
            res = os.path.join(os.path.dirname(args.O),res)
            print(res)
            air_files.append(res)
            os.system('xcrun -sdk macosx metal -c ' + _f + ' -o ' + res)
        spacer = " "
        os.system('xcrun -sdk macosx metallib ' + spacer.join(air_files) + ' -o ' + args.O)
    elif args.hlsl:
        # Only one file may be compiled at a time.
        if shutil.which("dxc") == None:
            print("dxc.exe not found in PATH. Exiting...")
            exit(1);
        
        target_profile = ""

        if args.shader_type == "vertex":
            target_profile = "vs_6_0"
        elif args.shader_type == "fragment":
            target_profile = "ps_6_0"
        elif args.shader_type == "compute":
            target_profile = "cs_6_0"

        os.system(f"dxc.exe -E ${args.entry_point} -T ${target_profile} -Fo ${args.O} ${args.files[0]}")

