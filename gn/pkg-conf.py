import argparse
import os
import sys 
import subprocess
import json

parser = argparse.ArgumentParser()
parser.add_argument("--name",type=str)
parser.add_argument("-L",dest="L",action="store_const",const=True,default=False) # List Libs
# parser.add_argument("-LD",dest="LD",action="store_action",const=True,default=False) # List Lib Dirs
# parser.add_argument("-I",dest="I",action="store_action",const=True,default=False) # List Includes
parser.add_argument("-F",dest="F",action="store_const",const=True,default=False) # Flags
args = parser.parse_args()

# data = subprocess.check_output(["pkg-config","--libs","--cflags",args.name])
opt = ""

if args.F:
    opt = "--cflags"
elif args.L:
    opt = "--libs"
p = subprocess.Popen("pkg-config " + opt + " " + args.name,shell=True,stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
line = p.stdout.readline()

__args = "{}".format(line).split(" ")
first = __args.pop(0)
n_first = first.split("'")
if len(n_first) > 1:
    __args.insert(0,n_first.pop())
__args.pop()

# __formatted_args = []
# for arg in __args:
#     __formatted_args.append('\"%s\"'% format(arg))

print(json.dumps(__args))



