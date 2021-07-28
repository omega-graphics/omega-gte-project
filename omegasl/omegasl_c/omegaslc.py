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

    parser.add_argument("--ebin",type=str,help=
    f"""
    Embed all binary shader files into single c source using omega-ebin
    """)

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

    if args.target == "metal":
        writer.outputStr = outstream
    
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
    
    if args.ebin:
        ebin_src = os.path.abspath(os.path.join(args.output,f"{f}__omegasl.ebin"))
        ebin = io.open(ebin_src,"w")
        ctxt.writeEBinConfig(ebin)
        ebin.close()
        embedded_bin = os.path.join(args.output,f'{f}.omegasl.src.c')
        cmd = os.path.abspath(args.ebin) + f" -i {ebin_src} -o {embedded_bin}"
        print(cmd)
        os.system(cmd)
        embedded_bin_is = io.open(os.path.join(args.output,f'{f}.omegasl.cc'),"w")
        embedded_bin_h_is = io.open(os.path.join(args.output,f'{f}.omegasl.h'),"w")
        ctxt.writeShaderLibEmbeddedBridge(f,embedded_bin_is,embedded_bin_h_is)
        embedded_bin_is.close()
        embedded_bin_h_is.close()
    else:
        ctxt.writeShaderMap()



# main()
    


