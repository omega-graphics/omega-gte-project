=======
OmegaSL
=======

    (Omega Shading Language)

OmegaSL is a cross-platform shading language designed to be used with OmegaGTE.

Syntax:
    StructDecl:
        Declares a data structure to be used on the cpu and the gpu unless declared internal (Struct suffixed with the keyword ``internal`` )

        .. code-block:: omegasl

            struct MyVertex {
                float3 pos;
                float4 color;
            };
            // Used only to transfer data between vertex and fragment shader stages.
            struct MyVertexRasterData internal {
                float4 pos : Position;
                float4 color;
            };

    ResourceDecl:
        Declares a resource to be allocated on the gpu. A resource is suffixed with a unique id (an unsigned 64-bit integer).
        This allows it to be allocated and be accessible to any shader throughout the duration of the program.
        A resource can only be of four of the following types:

            - buffer<struct T>
            - texture1d
            - texture2d
            - texture3d
            - sampler2d
            - sampler3d
            - uint

        Example:

        .. code-block:: omegasl

            struct MyData {
                float4 vec;
            };
            buffer<MyData> data : 0;
            texture2d tex : 1;

    ShaderDecl:
        Declares a shader routine to be executed on the gpu.
        They are prefixed with the keywords ``vertex``, ``fragment`` , or ``compute`` and in many other cases are prefixed with a resource map. (Array syntax placed before.)
        A resource map determines what gpu resources are accessible to the pipeline and what type of access the pipeline has.
        (Determines which type of shader is being declared).

        .. code-block:: omegasl

            struct RasterData internal {
                float4 pos : Position;
                float2 coord;
            };

            texture2d tex : 0;

            [in tex]
            fragment float4 fragShader(RasterData raster){
                sampler2d sampler;
                return sample(sampler,tex);
            }

Compilation:
    Via ``omegaslc``
        The main compiler for \*.omegasl sources.
        By default it outputs a \*.omegasllib file
        and an interface file (structs.h) in the output dir.

    Via
    .. cpp-class:: OmegaSLCompiler
        The runtime interface for handling compilation of OmegaSL shaders.



