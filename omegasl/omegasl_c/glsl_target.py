from .target import *

class GLSLTargetOpts(object):
    vulkan_semantics:bool
    def __init__(self,vulkan_semantics:bool):
        self.vulkan_semantics = vulkan_semantics
        return

    
class GLSLTarget(Target):
    """
    GLSL Target
    """
    out:TargetOutputContext
    opts:GLSLTargetOpts
    def __init__(self,out:TargetOutputContext,opts:GLSLTargetOpts):
        super().__init__(out,TargetType.GLSL)
        self.out = out 
        self.opts = opts
        return
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        pass
    
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        pass

    def beginBlock(self):
        self.out.write(" {\n")
        return
    def endBlock(self):
        self.out.write("};\n\n")
        return