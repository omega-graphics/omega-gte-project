from .glsl_target import *
from .hlsl_target import *
from .msl_target import *
from shutil import which

class TargetCompilerInvoker(object):
    """
    Invokes the native compiler for the corresponding targets
    """
    target:Target
    targetCtxt:TargetOutputContext
    def __init__(self,target:Target,targetCtxt:TargetOutputContext):
        self.target = target
        self.targetCtxt = targetCtxt
        return
    
    def check(self):
        if isinstance(self.target,HLSLTarget):
            hlsl_found = which("dxc")
            if hlsl_found is None:
                raise RuntimeError("HLSL compiler `dxc` was not found in PATH.")
        elif isinstance(self.target,MSLTarget):
            metal_found = which("xcrun")
            if metal_found is None:
                raise RuntimeError("Xcode is not installed.")
        elif isinstance(self.target,GLSLTarget):
            glslc_found = which("glslc")
            if glslc_found is None:
                raise RuntimeError("GLSL compiler `glslc` was not found in PATH")
        return

    def compile(self,inputFile:str,shaderType:str,output:str):
        if self.target == TargetType.METAL:
            os.system(f"xcrun -sdk macosx metal -c {inputFile} -o {output}")
            
        return

    def link(self,inputs:"list[str]",output:str):
        if self.target != TargetType.METAL:
            raise RuntimeError("Metal is the only target that supports linking")
        os.system(f"xcrun -sdk macosx metallib {' '.join(inputs)} -o {output}")