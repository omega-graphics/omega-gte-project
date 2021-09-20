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

Builtin-Functions:
    There are several builtin functions in the OmegaSL language.

    ``float2 make_float2(float x,float y)``
        Constructs a float2 vector.

    ``float2 make_float3(float x,float y,float z)
    float2 make_float3(float2 a,float z)``:
        Constructs a float3 vector.

    ``float2 make_float4(float x,float y,float z,float w)
    float2 make_float4(float2 a,float z,float w)
    float2 make_float4(float3 a,float w)``:
        Constructs a float4 vector.

    ``number dot(vec<number> a,vec<number> b)``:
       Calculates dot product of two vectors

    ``vec<number> cross(vec<number> a,vec<number> b)``:
       Calculates a cross product of two vectors.

    ``float4 sample(texture2d texture,float2 coord)``:
        Samples a texture2d and returns the color at the provided coord.

    ``float4 sample(texture3d texture,float3 coord)``:
        Samples a texture3d and returns the color at the provided coord.

Attributes:
    Render Pipeline Attributes:
        ``VertexID``:
            Defines the id of current vertex to draw.
        ``InstanceID``:
            Defines the id of current instance to draw.
        ``Position``:
            Defines the vertex position during the vertex stage in a render pass.
        ``Color``:
            Attributes a vector of 4 components to be used as a fragments color in a render pipeline.
        ``TexCoord``:
            Attributes a vector between 2 and 3 components to be used as coordinate for a 2D or 3D texture in a render pipeline.
    Compute Pipeline Attributes:
        ``GlobalThreadID``:
            The working thread's id in the total number of threads in a compute pipeline.
        ``ThreadGroupID``:
            The current threadgroup's id in the total number of thread groups dispatched in a pipeline.
        ``LocalThreadID``:
            The working thread's id in the total number of threads in its corresponding threadgroup.



Compilation:
    Via ``omegaslc``
        The main compiler for \*.omegasl sources.
        By default it outputs a \*.omegasllib file
        and an interface file (structs.h) in the output dir.

    Via
    .. cpp:class:: OmegaGTE::OmegaSLCompiler

        The runtime interface for handling compilation of OmegaSL shaders.



