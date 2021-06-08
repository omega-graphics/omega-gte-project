from .target import *

class MSLTargetOptions(object):
    use_simd:bool
    def __init__(self,use_simd:bool):
        self.use_simd = use_simd
        return

class MSLTarget(Target):
    """
    MSL Target
    """
    
    out:TargetOutputContext
    opts:MSLTargetOptions
    dist_file:str

    def __init__(self,out:TargetOutputContext,opts:MSLTargetOptions):
        super().__init__(out,TargetType.METAL)
        self.out = out
        (f,ext) = os.path.splitext(os.path.basename(self.out.source_file))
        self.dist_file = f"./{f}.metallib"
        self.opts = opts
        self.out.write("#include <metal_stdlib>\n")
        if self.opts.use_simd:
            self.out.write("#include <simd/simd.h>\n")
        self.out.write("\n")

    def convertTypeIfStandard(self,type:str) -> str:
        if self.opts.use_simd:
            if is_standard_type(type):
                return f"simd_{type}"
        return type

    def writeType(self,name:str):
        self.out.write(self.convertTypeIfStandard(name))
        return

    def annotationToString(self,n:ast.AnnAssign) -> str:
        ty = n.annotation
        res_name = n.target.id
        if isinstance(ty,ast.Subscript):
           # ConstBuffer Convert
            _name = ty.value.id 
            if _name == "ConstBuffer":
                arg_tuple:ast.Tuple = ty.slice
                if len(arg_tuple.elts) != 2:
                    raise RuntimeError("ConstBuffer requires 2 and ONLY 2 arguments.\nUsage:\nConstBuffer[Object,GPUIndex]")
                # Assume ast.Name
                
                buffer_object:str =  arg_tuple.elts[0].id
                gpu_index:str = str(arg_tuple.elts[1].value)
                return f"constant {buffer_object} * {res_name}[[buffer({gpu_index})]]"
            
            # Texture (1D/2D/3D) Convert 

            elif _name == "Texture2D":
                arg_tuple:ast.Tuple = ty.slice
                if len(arg_tuple.elts) != 2:
                    raise RuntimeError("Textured2D requires 2 and ONLY 2 arguments.\nUsage:\nTextured2D[CoordNumType,GPUIndex]")
                texture_coordinate_ty:str =  arg_tuple.elts[0].id
                gpu_index:str = str(arg_tuple.elts[1].value)

                return f"texture2d<{texture_coordinate_ty}> {res_name}[[texture({gpu_index})]]"
                
    
        

        return "BLANK"
    def writeStructPropDecl(self,name:str,type:str):
        metal_attribute:str
        metal_native_type:str
        if type == "Position":
            metal_native_type = self.convertTypeIfStandard("float4")
            metal_attribute = "position"
        else:
            metal_attribute = None 
            metal_native_type = self.convertTypeIfStandard(type)
        if metal_attribute is not None:
            self.out.write(f"  {metal_native_type} {name} [[{metal_attribute}]];\n")
        else:
            self.out.write(f"  {metal_native_type} {name};\n")
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.out.write(f"vertex {self.convertTypeIfStandard(type_str)} {name} (")
        for k in params:
            self.out.write(params[k])
            self.out.write(", ")
        self.out.write("unsigned vertexID [[vertex_id]]")
        self.out.write(")")

       
        self.out.writeShaderEntryToMap(self.dist_file,utf8str_to_bytes(name))
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.out.write(f"fragment {self.convertTypeIfStandard(type_str)} {name} (")
        
        in_stage = params["input"]
        self.out.write(in_stage)
        self.out.write(" in [[stage_in]]")
        
        for k in params:
            if k == "input":
                continue
            self.out.write(", ")
            self.out.write(params[k])
        self.out.write(")")
    def beginBlock(self):
        self.out.write(" {\n")
        return
    def endBlock(self):
        self.out.write("};\n\n")
        return