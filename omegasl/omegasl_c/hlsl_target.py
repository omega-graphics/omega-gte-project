from .target import *

class HLSLTarget(Target):
    """
    HLSL Target
    """

    out:TargetOutputContext

    def __init__(self,out:TargetOutputContext):
        super().__init__(out,TargetType.HLSL)
        self.out = out
    def writeType(self,name:str):
        self.out.write(name)
    def writeStructPropDecl(self,name:str,type:str):
        hlsl_annotation:str
        hlsl_native_type:str
        if type == "Position":
            hlsl_native_type = "float4"
            hlsl_annotation = "SV_POSITION"
        elif type == "TextureCoord":
            hlsl_native_type = "float4"
            hlsl_annotation = "TEXCOORD"
        else:
            hlsl_native_type = type
            hlsl_annotation = None 

        if hlsl_annotation is None:
            self.out.write(f"  {hlsl_native_type} {name};\n")
        else:
            self.out.write(f"  {hlsl_native_type} {name} : {hlsl_annotation};\n")
        return
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.out.write(f"{type_str} {name} (")
        for k in params:
            self.out.write(params[k])
            self.out.write(" ")
            self.out.write(k)
        self.out.write(")")
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.out.write(f"{type_str} {name} (")
        for k in params:
            self.out.write(params[k])
            self.out.write(" ")
            self.out.write(k)
        self.out.write(") : SV_TARGET")
    def beginBlock(self):
        self.out.write("{\n")
        return
    def endBlock(self):
        self.out.write("\n};\n\n")
        return