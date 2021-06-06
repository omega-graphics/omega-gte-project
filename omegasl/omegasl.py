import os,json,ast,io,argparse

class Target(object):
    """
    The interface for all Targets
    """
    os:io.TextIOWrapper

    def __init__(self,os:io.TextIOWrapper):
        self.os = os
    
    def writeStructDecl(self,name:str):
        self.os.write(f"struct {name}")
        return
    def writeStructPropDecl(self,name:str,type:str):
        hlsl_annotation:str
        hlsl_native_type:str
        if type == "Position":
            hlsl_native_type = "float4"
            hlsl_annotation = "SV_POSITION"
        self.os.write(f"  {hlsl_native_type} {name} : {hlsl_annotation};\n")
        return
    def writeVertexShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"):
        pass
    def writeFragmentShaderDecl(self,name:str,type_str:str,params:"dict[str,str]"):
        pass
    def beginBlock(self):
        pass
    def endBlock(self):
        pass

class HLSLTarget(Target):

    os:io.TextIOWrapper

    def __init__(self,os:io.TextIOWrapper):
        super().__init__(os)
        self.os = os
    """
    HLSL Target
    """
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
        self.os.write(")")
    def beginBlock(self):
        self.os.write("{\n")
        return
    def endBlock(self):
        self.os.write("\n}\n\n")
        return

class MSLTarget(Target):
    """
    MSL Target
    """
    pass 

class GLSLTarget(Target):
    """
    GLSL Target
    """
    pass 

class TargetWriter(object):
    outputStr : io.TextIOWrapper
    target: Target
    def __init__(self,t:Target):
        self.target = t 
        return 
    def writeExpr(self,expr:ast.expr):
        return
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
            print(prop)
        self.target.endBlock()

        return
    def writeShaderFunction(self,func:ast.FunctionDef):
        decorator_count = len(func.decorator_list)
        if decorator_count  == 0 or decorator_count > 1:
            raise RuntimeError("A Shader Function must have one and ONLY one type")

        shader_type:str = func.decorator_list[0].id
        ret:ast.Name = func.returns
        if ret is None:
            raise RuntimeError("OmegaSLPy requires that all shaders must be type annotated")

        params:"dict[str,str]" = {}
        for arg in func.args.args:
            if arg.annotation is None:
                raise RuntimeError("OmegaSLPy requires that all shaders must be type annotated")
            params[arg.arg] = arg.annotation.id
        
        if shader_type == "vertex":
            self.target.writeVertexShaderDecl(func.name,type_str=ret.id,params=params)
        elif shader_type == "fragment":
            self.target.writeFragmentShaderDecl(func.name,type_str=ret.id,params=params)
        self.target.beginBlock()
        self.target.endBlock()

def main():
    parser = argparse.ArgumentParser(prog="omegasl")
    parser.add_argument("--target",type=str)
    parser.add_argument("file",type=str)
    args = parser.parse_args()

    file:str = args.file

    (f,ext) = os.path.splitext(file)

    out = f"{f}.hlsl"

    ostream = io.open(out,"w")
    
    istream = io.open(args.file,"r")

    t:Target
    if args.target == "hlsl":
        t = HLSLTarget(os=ostream)
    elif args.target == "metal":
        t = MSLTarget(os=ostream)

    writer = TargetWriter(t=t)
    
    m = ast.parse(istream.read(),args.file)

    for stmt in m.body:
        if isinstance(stmt,ast.FunctionDef):
            writer.writeShaderFunction(stmt)
        elif isinstance(stmt,ast.ClassDef):
            writer.writeStruct(stmt)
            
           


main()
    


