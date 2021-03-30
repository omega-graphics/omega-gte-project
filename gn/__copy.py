import sys
import os
import shutil
import argparse
# import PyUtils

# if(PyUtils.is_python_3()):
#     import shutil
# else:
#     import os

parser = argparse.ArgumentParser()

parser.add_argument("--src",type=str)
parser.add_argument("--dest",type=str)
args = parser.parse_args()
src = args.src
dest = args.dest


if(sys.api_version > 2.9):
    if(os.path.isfile(src)):
        if(os.path.exists(dest)):
            os.remove(dest)
        
        shutil.copyfile(src,dest)
    elif(os.path.isdir(src)):
        if(os.path.exists(dest)):
            shutil.rmtree(dest)
        
        shutil.copytree(src,dest,True)
else:
    os.system("cp " + src + " " + dest)