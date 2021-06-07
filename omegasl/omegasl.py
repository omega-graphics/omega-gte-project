import os,json,ast,io,argparse
from queue import Queue
from enum import Enum


"""

Standard Resource Types:

ConstBuffer[T,I]
Buffer[T,I]
Texture2D[T,I]
Texture3D[T,I]
Texture1D[T,I]

Std Data Types:

float
float2
float3
float4

float1x2
float2x2
float3x2
float4x2

float1x3
float2x3
float3x3
float4x3

float1x4
float2x4
float3x4
float4x4


"""


def is_standard_type(ty:str) -> bool:
    return ty == "float" or ty == "float2" or ty == "float3" or ty == "float4"

class TargetType(Enum):
    HLSL = 0,
    METAL = 1,
    GLSL = 2

target:TargetType

class Target(object):
    """
    The interface for all Targets
    """
    os:io.TextIOWrapper

    def __init__(self,os:io.TextIOWrapper):
        self.os = os

    def writeIdentifier(self,name:str):
        self.os.write(name)
        pass
    
    def writeType(self,name:str):
        self.os.write(name)
        return
    def writeOperator(self,op:ast.operator):
        if isinstance(op,ast.Add):
            self.os.write(" + ")
        elif isinstance(op,ast.Sub):
            self.os.write(" - ")
        elif isinstance(op,ast.Mult):
            self.os.write(" * ")
        elif isinstance(op,ast.Div):
            self.os.write(" / ")
        pass 

    def writeLogicOperator(self,op:ast.boolop):
        if isinstance(op,ast.And):
            self.os.write(" && ")
        elif isinstance(op,ast.Or):
            self.os.write(" || ")
        elif isinstance(op,ast.Not):
            self.os.write(" ! ")
        
    def convertTypeIfStandard(type:str):
        pass
    def writeStructDecl(self,name:str):
        self.os.write(f"struct {name}")
        return
    def annotationToString(self,n:ast.AnnAssign) -> str:
        pass
    def writeStructPropDecl(self,name:str,type:str):
        pass
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"):
        pass
    def writeVertexShaderReturn(self):
        self.os.write("return ")
        pass
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"):
        pass
    def writeFragmentShaderReturn(self):
        self.os.write("return ")
        pass
    def beginBlock(self):
        pass
    def endBlock(self):
        pass

class HLSLTarget(Target):
    """
    HLSL Target
    """

    os:io.TextIOWrapper

    def __init__(self,os:io.TextIOWrapper):
        super().__init__(os)
        self.os = os
    def writeType(self,name:str):
        self.os.write(name)
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
            self.os.write(f"  {hlsl_native_type} {name};\n")
        else:
            self.os.write(f"  {hlsl_native_type} {name} : {hlsl_annotation};\n")
        return
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.os.write(f"{type_str} {name} (")
        for k in params:
            self.os.write(params[k])
            self.os.write(" ")
            self.os.write(k)
        self.os.write(")")
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.os.write(f"{type_str} {name} (")
        for k in params:
            self.os.write(params[k])
            self.os.write(" ")
            self.os.write(k)
        self.os.write(") : SV_TARGET")
    def beginBlock(self):
        self.os.write("{\n")
        return
    def endBlock(self):
        self.os.write("\n};\n\n")
        return

class MSLTargetOptions(object):
    use_simd:bool
    def __init__(self,use_simd:bool):
        self.use_simd = use_simd
        return

class MSLTarget(Target):
    """
    MSL Target
    """
    
    os:io.TextIOWrapper
    opts:MSLTargetOptions

    def __init__(self,os:io.TextIOWrapper,opts:MSLTargetOptions):
        super().__init__(os)
        self.os = os
        self.opts = opts
        self.os.write("#include <metal_stdlib>\n")
        if self.opts.use_simd:
            self.os.write("#include <simd/simd.h>\n")
        self.os.write("\n")

    def convertTypeIfStandard(self,type:str) -> str:
        if self.opts.use_simd:
            if is_standard_type(type):
                return f"simd_{type}"
        return type

    def writeType(self,name:str):
        self.os.write(self.convertTypeIfStandard(name))
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
            self.os.write(f"  {metal_native_type} {name} [[{metal_attribute}]];\n")
        else:
            self.os.write(f"  {metal_native_type} {name};\n")
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.os.write(f"vertex {self.convertTypeIfStandard(type_str)} {name} (")
        for k in params:
            self.os.write(params[k])
            self.os.write(", ")
        self.os.write("unsigned vertexID [[vertex_id]]")
        self.os.write(")")
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"): 
        self.os.write(f"fragment {self.convertTypeIfStandard(type_str)} {name} (")
        
        in_stage = params["input"]
        self.os.write(in_stage)
        self.os.write(" in [[stage_in]]")
        
        for k in params:
            if k == "input":
                continue
            self.os.write(", ")
            self.os.write(params[k])
        self.os.write(")")
    def beginBlock(self):
        self.os.write(" {\n")
        return
    def endBlock(self):
        self.os.write("};\n\n")
        return

class GLSLTarget(Target):
    """
    GLSL Target
    """
    pass 


class TargetWriter(object):

    outputStr : io.TextIOWrapper

    target: Target

    resourceQueue : list[ast.AnnAssign]

    declared_vars: list[str]

    def __init__(self,t:Target):
        self.target = t 
        self.resourceQueue = []
        self.outputStr = t.os
        return 

    def resourceExists(self,name:str):
        for r in self.resourceQueue:
            res_node:ast.Name = r.target
            if res_node.id == name:
                return r
        return None

    def writeShaderResourceDecl(self,expr:ast.AnnAssign):
        res_name:ast.Name = expr.target
        if len(self.resourceQueue) > 0:
            if self.resourceExists(res_name.id) is not None:
                raise RuntimeError("Resources must have unique names")
        self.resourceQueue.append(expr)
        return

    def writeDecl(self,decl:ast.stmt,funcContext:str):
        """
        Writes a Decl to Output Stream
        (Must be defined only in a function or shader)
        """
        # VarDecl
        if isinstance(decl,ast.AnnAssign):
            var_ty = decl.annotation
            var_name = decl.target

            if not isinstance(var_name,ast.Name):
                raise RuntimeError("AST NAME EXPECTED!!")

            if not isinstance(var_ty,ast.Name):
                raise RuntimeError("AST NAME EXPECTED!!")
            
            if not is_standard_type(var_ty.id):
                self.target.writeIdentifier(var_ty.id)
            else:
                self.target.writeType(var_ty.id)

            self.outputStr.write(" ")
            self.target.writeIdentifier(var_name.id)
            if decl.value is not None:
                self.outputStr.write(" = ")
                self.writeExpr(decl.value)
            return
        # CondDecl
        elif isinstance(decl,ast.If):
            return
        # ReturnDecl
        elif isinstance(decl,ast.Return):
            
            if funcContext == "fragment":
                if decl.value is None:
                    raise RuntimeError("Fragment shader must return a value (Specifically a float4)")

                self.target.writeFragmentShaderReturn()
                self.writeExpr(decl.value)
            elif funcContext == "vertex":
                self.target.writeVertexShaderReturn()
                if decl.value is None:
                    raise RuntimeError("Vertex shader must return a value!")
                self.writeExpr(decl.value)
            return
        return

    def writeExpr(self,expr:ast.expr):
        """
        Writes an AST Expr to the Output Stream.
        (Does not include VarDecls)
        """
        if isinstance(expr,ast.Constant):
            self.outputStr.write(str(expr.value))
            return
        elif isinstance(expr,ast.Name):
            self.target.writeIdentifier(expr.id)
        # Math Operations
        elif isinstance(expr,ast.BinOp):

            self.writeExpr(expr.left)
            self.target.writeOperator(expr.op)
            self.writeExpr(expr.right)

            return
        # Bool Operations
        elif isinstance(expr,ast.BoolOp):
            self.target.writeLogicOperator(expr.op)
        # Function Invocation
        elif isinstance(expr,ast.Call):
            func_n:ast.Name = expr.func
            self.outputStr.write(func_n.id)
            self.outputStr.write("(")
            for i in range(len(expr.args)):
                if i > 0:
                    self.outputStr.write(",")
                self.writeExpr(expr.args[i])
            self.outputStr.write(")")
            return
        
        elif isinstance(expr,ast.List):
            # Write C Array
            self.outputStr.write("{")
            for i in range(len(expr.elts)):
                if i > 0:
                    self.outputStr.write(", ")
                self.writeExpr(expr.elts[i])
            self.outputStr.write("}")

        elif isinstance(expr,ast.Assign):
            self.writeExpr(expr.targets[0])
            self.outputStr.write(" = ")
            self.writeExpr(expr)

        elif isinstance(expr,ast.Attribute):
            self.writeExpr(expr.value)
            self.outputStr.write(".")
            self.outputStr.write(expr.attr)

        # return

    def writeStruct(self,struct:ast.ClassDef):
        decorator_count = len(struct.decorator_list)
        if decorator_count == 0 or decorator_count > 1:
            raise RuntimeError("All classes defined in OmegaSLPy must be annotated with the 'struct' decorator")

        class_name = struct.name
        self.target.writeStructDecl(class_name)
        self.target.beginBlock()

        for prop in struct.body:
            if isinstance(prop,ast.AnnAssign):
                prop_name:ast.Name = prop.target
                prop_type:ast.Name = prop.annotation
                self.target.writeStructPropDecl(prop_name.id,prop_type.id)
            else:
                raise RuntimeError("Only Variable Decls are allowed")
            # print(prop)
        self.target.endBlock()

        return
    def writeShaderFunction(self,func:ast.FunctionDef):
        decorator_count = len(func.decorator_list)
        if decorator_count  == 0 or decorator_count > 1:
            raise RuntimeError("A Shader Function must have one and ONLY one type")

        func_decor:ast.Call = func.decorator_list[0]
        shader_type:str = func_decor.func.id

        params:"dict[str,str]" = {}

        for a in func_decor.args:
            if not isinstance(a,ast.Name):
                raise RuntimeError("Only identifiers are allowed in this context")

            arg:ast.Name = a
            res = self.resourceExists(arg.id)
            if res is None:
                raise RuntimeError(f"Resource {arg.id} does not exist")
            if target == TargetType.METAL:
                t = self.target.annotationToString(res)
                params[arg.id] = t

        ret:ast.Name = func.returns
        if ret is None:
            raise RuntimeError("OmegaSLPy requires that all shaders must be type annotated")

        
        for arg in func.args.args:
            if arg.annotation is None:
                raise RuntimeError("OmegaSLPy requires that all shaders must be type annotated")
            params[arg.arg] = arg.annotation.id
        
        if shader_type == "vertex":
            self.target.writeVertexShaderDecl(func.name,type_str=ret.id,params=params)
        elif shader_type == "fragment":
            if params.get("input") is None:
                raise RuntimeError("There must be only 1 argument to a fragment function")
            self.target.writeFragmentShaderDecl(func.name,type_str=ret.id,params=params)
        self.target.beginBlock()

        for stmt in func.body:
            self.outputStr.write("  ")
            if isinstance(stmt,ast.Expr):
                self.writeExpr(stmt.value)
            else:
                self.writeDecl(stmt,shader_type)
            self.outputStr.write(";\n")
        self.target.endBlock()


file_header = "// WARNING: This File was generated by omegaslc. DO NOT EDIT!\n\n"

def main():
    global target
    parser = argparse.ArgumentParser(prog="omegasl")

    parser.add_argument("--target",type=str,choices=["hlsl","metal","glsl"],help=
    """
    The target shading language to compile to

    Options:
      hlsl -> Compile to HLSL
      metal -> Compile to Metal Shading Language
      glsl -> Compile to GLSL
    """)

    metal_opts = parser.add_argument_group("Metal Target Options:")

    metal_opts.add_argument("--use-simd",action="store_const",const=True,default=False,help=
    """
    Use simd library matricies instead of the builtin matrices.
    """)

    parser.add_argument("file",type=str)

    args = parser.parse_args()

    file:str = args.file

    (f,ext) = os.path.splitext(file)

    ostream:io.TextIOWrapper
    
    istream = io.open(args.file,"r")

    t:Target
    if args.target == "hlsl":
        ostream = io.open(f"{f}.hlsl","w")
        ostream.write(file_header)
        t = HLSLTarget(os=ostream)
        target = TargetType.HLSL
    elif args.target == "metal":
        ostream = io.open(f"{f}.metal","w")
        ostream.write(file_header)
        t = MSLTarget(os=ostream,opts=MSLTargetOptions(args.use_simd))
        target = TargetType.METAL
    

    writer = TargetWriter(t=t)
    
    m = ast.parse(istream.read(),args.file)

    for stmt in m.body:
        if isinstance(stmt,ast.FunctionDef):
            writer.writeShaderFunction(stmt)
        elif isinstance(stmt,ast.ClassDef):
            writer.writeStruct(stmt)
        elif isinstance(stmt,ast.AnnAssign):
            writer.writeShaderResourceDecl(stmt)
            
           


main()
    


