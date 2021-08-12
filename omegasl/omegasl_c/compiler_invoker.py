from .glsl_target import *
from .hlsl_target import *
from .msl_target import *
from shutil import which


class TargetCompilerInvoker:
    """
    Invokes the native compiler for the corresponding targets
    """
    target: Target
    targetCtxt: TargetOutputContext
    def __init__(self,target:Target,targetCtxt:TargetOutputContext):
        self.target = target
        self.targetCtxt = targetCtxt
        return
    
    def check(self):
        if self.target.type == TargetType.HLSL:
            hlsl_found = which("dxc")
            if hlsl_found is None:
                raise RuntimeError("HLSL compiler `dxc` was not found in PATH.")
        elif self.target.type == TargetType.METAL:
            metal_found = which("xcrun")
            if metal_found is None:
                raise RuntimeError("Xcode is not installed.")
        elif self.target.type == TargetType.GLSL:
            glslc_found = which("glslc")
            if glslc_found is None:
                raise RuntimeError("GLSL compiler `glslc` was not found in PATH")
        return

    def compile(self,inputFile:str,shaderName:str,shaderProfile:str,output:str):
        if self.target.type == TargetType.HLSL:
            os.system(f"dxc -nologo -Od -Fo{output} -E{shaderName} -T{shaderProfile} {inputFile}")
        elif self.target.type == TargetType.METAL:
            os.system(f"xcrun -sdk macosx metal -c {inputFile} -o {output}")
        elif self.target.type == TargetType.GLSL:
            os.system(f"glslc -c --target-env=vulkan1.2 -o {output} -c {inputFile}")
        return

    def link(self,inputs:"list[str]",output:str):
        if self.target.type != TargetType.METAL:
            raise RuntimeError("Metal is the only target that supports linking")
        os.system(f"xcrun -sdk macosx metallib {' '.join(inputs)} -o {output}")