from .target import *

class HLSLTarget(Target):
    """
    HLSL Target
    """

    out:TargetOutputContext
    shaders:"dict[str,ShaderType]"

    def __init__(self,out:TargetOutputContext):
        super().__init__(out,type=TargetType.HLSL)
        self.out = out
        self.shaders = {}
    def writeType(self,name:str):
        self.out.write(name)
    def writeStructPropDecl(self,name:str,type:str):
        hlsl_annotation:str
        hlsl_native_type:str
        if type == "Position":
            hlsl_native_type = "float4"
            hlsl_annotation = "POSITION"
        elif type == "TextureCoord":
            hlsl_native_type = "float4"
            hlsl_annotation = "TEXCOORD"
        elif type == "Color":
            hlsl_native_type = "float4"
            hlsl_annotation = "COLOR"
        else:
            hlsl_native_type = type
            hlsl_annotation = None 

        if hlsl_annotation is None:
            self.out.write(f"  {hlsl_native_type} {name};\n")
        else:
            self.out.write(f"  {hlsl_native_type} {name} : {hlsl_annotation};\n")
        return
    def annotationToString(self, n: ast.AnnAssign) -> str:
        res_name:ast.Name = n.target
        if not isinstance(res_name,ast.Name):
            raise RuntimeError("Expected AST NAME!")
        
        ty:ast.Subscript = n.annotation
        type_args:ast.Tuple = ty.slice
        type_name:str = ty.value.id

        # Assume is ast.Name
        type_arg:str = type_args.elts[0].id
        # Assume is ast.Constant
        gpu_index:int = type_args.elts[1].value
        output_ty:str
        if type_name == "ConstBuffer" or type_name == "Buffer":
            output_ty = "StructuredBuffer"

        return f"{output_ty}<{type_arg}> {res_name.id} : register(t{gpu_index});\n"
        
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.shaders[name] = ShaderType.VERTEX
        self.out.write(f"{type_str} {name} (")
        self.out.write(f"uint vertexID : SV_VertexID")
        for k in params:
            self.out.write(",")
            self.out.write(params[k])
            self.out.write(" ")
            self.out.write(k)
        self.out.write(")")
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"):
        self.shaders[name] = ShaderType.COMPUTE 
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