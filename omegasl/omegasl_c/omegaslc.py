from .glsl_target import *
from .msl_target import *
from .hlsl_target import *

from .target_writer import *
from .compiler_invoker import *

import argparse




def main():
    parser = argparse.ArgumentParser(prog="omegaslc",description=
    """
    A cross platform shading language transpiler for HLSL (Direct3D 12), MSL (Metal), and GLSL (Vulkan and OpenGL)
    """,usage="omegaslc [options] file")

    parser.add_argument("--temp",type=str,help=
    f"""
    The temp directory to write temp files to.
    """)

    parser.add_argument("--output","-o",type=str,dest="output",help=
    f"""
    The output directory to write the output files to.
    """)

    parser.add_argument("--target",type=str,choices=["hlsl","metal","glsl"],help=
    f"""
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

    (f,ext) = os.path.splitext(os.path.basename(file))

    temp_file = os.path.join(args.temp,f)


    shaderMapFile = os.path.join(args.output,f"{f}.shadermap")

    outstream:io.TextIOWrapper

    istream = io.open(args.file,"r")

    ctxt = TargetOutputContext()
    ctxt.output_dir = args.output
    ctxt.source_file = args.file
    ctxt.temp_dir = args.temp

    if not os.path.exists(args.output):
        os.mkdir(args.output)
    
    if not os.path.exists(args.temp):
        os.mkdir(args.temp)

    ctxt.shaderMapOut = io.open(shaderMapFile,"wb")

    

    t:Target
    if args.target == "hlsl":
        # outstream = io.open(f"{temp_file}.hlsl","w")
        # outstream.write(file_header)

        # ctxt.out = outstream
        t = HLSLTarget(out=ctxt)

        compiler = TargetCompilerInvoker(target=t,targetCtxt=ctxt)
    
        
    elif args.target == "metal":

        outstream = io.open(f"{temp_file}.metal","w")
        outstream.write(file_header)
        ctxt.out = outstream
        t = MSLTarget(out=ctxt,opts=MSLTargetOptions(args.use_simd))

        compiler = TargetCompilerInvoker(target=t,targetCtxt=ctxt)
        
       
    

    writer = TargetWriter(t=t)
    
    m = ast.parse(istream.read(),args.file)

    for stmt in m.body:
        if isinstance(stmt,ast.FunctionDef):

            writer.writeShaderFunction(stmt)
        elif isinstance(stmt,ast.ClassDef):
            writer.writeStruct(stmt)
        elif isinstance(stmt,ast.AnnAssign):
            writer.writeShaderResourceDecl(stmt)

    if args.target == "metal":
        outstream.close()
    
    t.finish()
    compiler.check()

    if  args.target == "hlsl":
        compiler.check()

        for shader in t.shaders:
            shader_ty:ShaderType = t.shaders[shader]
            
            if shader_ty == ShaderType.VERTEX:
                profile = "vs_6_0"
            elif shader_ty == ShaderType.FRAGMENT:
                profile = "ps_6_0"
            elif shader_ty == ShaderType.COMPUTE:
                profile = "cs_6_0"

            compiler.compile(inputFile=os.path.join(f"{args.temp}",f"{shader}.hlsl"),shaderName=shader,shaderProfile=profile,output=os.path.join(args.output,f"{shader}.cso"))
    elif args.target == "metal":     
        compiler.compile(inputFile=f"{temp_file}.metal",shaderName="NULL",shaderProfile="",output=f"{temp_file}.air")
        compiler.link(inputs=[f"{temp_file}.air"],output=os.path.join(args.output,f"{f}.metallib"))
    ctxt.writeShaderMap()

main()
    


