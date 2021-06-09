from enum import Enum 
import io,ast,os
# from shutil import which



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

def utf8str_to_bytes(s:str) -> bytes:
    return bytes(s,"utf8")

class TargetType(Enum):
    HLSL = 0,
    METAL = 1,
    GLSL = 2

class ShaderType(Enum):
    VERTEX = 0,
    FRAGMENT = 1,
    COMPUTE = 2


class TargetOutputContext(object):
    source_file:str
    output_dir:str
    b:dict[str,list[bytes]]
    def __init__(self):
        self.b = {}
        return
    out:io.TextIOWrapper
    shaderMapOut:io.BufferedWriter

    def write(self,content:str):
        self.out.write(content)
    def writeShaderMap(self):
       l = len(self.b)
       self.shaderMapOut.write(bytes(l))
       for e in self.b:
           self.shaderMapOut.write(utf8str_to_bytes(e))
           self.shaderMapOut.write(bytes(len(self.b[e])))
           for shaderName in self.b[e]:
               self.shaderMapOut.write(shaderName)
    def writeShaderEntryToMap(self,filename:str,data:bytes):
        if self.b.get(filename) is None:
            self.b[filename] = []
        self.b[filename].append(data)
   

class Target(object):
    """
    The interface for all Targets
    """
    out:TargetOutputContext
    type:TargetType

    def __init__(self,out:TargetOutputContext,type:TargetType):
        self.out = out
        self.type = type

    def writeIdentifier(self,name:str):
        self.out.write(name)
        pass
    
    def writeType(self,name:str):
        self.out.write(name)
        return
    def writeOperator(self,op:ast.operator):
        if isinstance(op,ast.Add):
            self.out.write(" + ")
        elif isinstance(op,ast.Sub):
            self.out.write(" - ")
        elif isinstance(op,ast.Mult):
            self.out.write(" * ")
        elif isinstance(op,ast.Div):
            self.out.write(" / ")
        pass 

    def writeLogicOperator(self,op:ast.boolop):
        if isinstance(op,ast.And):
            self.out.write(" && ")
        elif isinstance(op,ast.Or):
            self.out.write(" || ")
        elif isinstance(op,ast.Not):
            self.out.write(" ! ")
        
    def convertTypeIfStandard(type:str):
        pass
    def writeStructDecl(self,name:str):
        self.out.write(f"struct {name}")
        return
    def annotationToString(self,n:ast.AnnAssign) -> str:
        pass
    def writeStructPropDecl(self,name:str,type:str):
        pass
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"):
        pass
    def writeVertexShaderReturn(self):
        self.out.write("return ")
        pass
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"):
        pass
    def writeFragmentShaderReturn(self):
        self.out.write("return ")
        pass
    def writeComputeShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"):
        pass
    def writeComputeShaderReturn(self,name:str,type_str:str,params:"dict[str,str]"):
        self.out.write("return ")
        pass
    def beginBlock(self):
        pass
    def endBlock(self):
        pass
    


