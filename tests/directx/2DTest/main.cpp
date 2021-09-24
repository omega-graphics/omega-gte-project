#include <OmegaGTE.h>
#include <windows.h>
#include <pathcch.h>

#pragma comment(lib,"Pathcch.lib")

LRESULT CALLBACK   WndProc(HWND, UINT, WPARAM, LPARAM);

#define VERTEX_SHADER "vertexFunc"
#define FRAGMENT_SHADER "fragFunc"


static OmegaGTE::GTE gte;
static OmegaGTE::SharedHandle<OmegaGTE::GTEShaderLibrary> library;
static OmegaGTE::SharedHandle<OmegaGTE::GENativeRenderTarget> renderTarget;
static OmegaGTE::SharedHandle<OmegaGTE::OmegaTessalationEngineContext> tessContext;
static OmegaGTE::SharedHandle<OmegaGTE::GERenderPipelineState> renderPipelineState;
static OmegaGTE::SharedHandle<OmegaGTE::GEBuffer> vertexBuffer;
static OmegaGTE::SharedHandle<OmegaGTE::GEBufferWriter> bufferWriter;

void formatGPoint3D(std::ostream & os,OmegaGTE::GPoint3D & pt){
    os << "{ x:" << pt.x << ", y:" << pt.y << ", z:" << pt.z << "}";
};

void writeVertex(OmegaGTE::GPoint3D & pt,OmegaGTE::FVec<4> & color){
    auto vertex_pos = OmegaGTE::FVec<4>::Create();
    vertex_pos[0][0] = pt.x;
    vertex_pos[1][0] = pt.y;
    vertex_pos[2][0] = pt.z;
    vertex_pos[3][0] = 1.f;

    bufferWriter->structBegin();
    bufferWriter->writeFloat4(vertex_pos);
    bufferWriter->writeFloat4(color);
    bufferWriter->structEnd();
}

void tessalate(){

    OmegaGTE::GRect rect;
    rect.h = 300;
    rect.w = 300;
    rect.pos.x = 0;
    rect.pos.y = 0;
    auto rect_mesh = tessContext->tessalateSync(OmegaGTE::TETessalationParams::Rect(rect));

    std::cout << "Tessalated GRect" << std::endl;
    auto color = OmegaGTE::makeColor(1.f,0.f,0.f,1.f);

    std::cout << "Created Matrix GRect" << std::endl;

    OmegaGTE::BufferDescriptor bufferDescriptor {OmegaGTE::BufferDescriptor::Upload,6 * (FLOAT4_SIZE + FLOAT4_SIZE),FLOAT4_SIZE + FLOAT4_SIZE};

    vertexBuffer = gte.graphicsEngine->makeBuffer(bufferDescriptor);

    bufferWriter->setOutputBuffer(vertexBuffer);

    for(auto & mesh : rect_mesh.meshes){
        std::cout << "Mesh 1:" << std::endl;
        for(auto &tri : mesh.vertexTriangles){
            std::ostringstream ss;
            ss << "Triangle: {\n  A:";
            formatGPoint3D(ss,tri.a);
            ss << "\n  B:";
            formatGPoint3D(ss,tri.b);
            ss << "\n  C:";
            formatGPoint3D(ss,tri.c);
            ss << "\n}";
            std::cout << ss.str() << std::endl;
            std::cout << "Create Vertex" << std::endl;
            writeVertex(tri.a,color);
            writeVertex(tri.b,color);
            writeVertex(tri.c,color);
        };
    };

    bufferWriter->finish();

};


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
    WCHAR name[MAX_PATH];
    
    GetModuleFileNameW(hInstance,name,MAX_PATH);
    PathCchRemoveExtension(name,MAX_PATH);
    PathCchRemoveFileSpec(name,MAX_PATH);
    
    SetCurrentDirectoryW(name);
    MessageBoxW(GetForegroundWindow(),(std::wstring(L"Current Dir:") + name).c_str(),L"NOTE",MB_OK);

    gte = OmegaGTE::Init(nullptr);


    library = gte.graphicsEngine->loadShaderLibrary("./shaders.omegasllib");

    bufferWriter = OmegaGTE::GEBufferWriter::Create();

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = "Test";
    wcex.hIconSm        = NULL;

    ATOM a = RegisterClassEx(&wcex);

    UINT dpi = GetDpiFromDpiAwarenessContext(GetThreadDpiAwarenessContext());

    FLOAT scaleFactor = FLOAT(dpi)/96.f;


    HWND hwnd = CreateWindowA(MAKEINTATOM(a),"",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,500 * scaleFactor,500 * scaleFactor,NULL,NULL,hInstance,NULL);
    if(!IsWindow(hwnd)){
         MessageBoxA(GetForegroundWindow(),"Failed to Create Window","NOTE",MB_OK);
        exit(1);
    };

    MessageBoxA(GetForegroundWindow(),"App Pre Launch -- Stage 0","NOTE",MB_OK);
    MessageBoxA(GetForegroundWindow(),"App Pre Launch -- Stage 1","NOTE",MB_OK);

    OmegaGTE::NativeRenderTargetDescriptor renderTargetDesc {};

    renderTargetDesc.hwnd = hwnd;
    renderTargetDesc.isHwnd = true;

    OmegaGTE::RenderPipelineDescriptor pipelineDesc;
    pipelineDesc.rasterSampleCount = 0;
    pipelineDesc.vertexFunc = library->shaders[VERTEX_SHADER];
    pipelineDesc.fragmentFunc = library->shaders[FRAGMENT_SHADER];
    pipelineDesc.depthAndStencilDesc.enableDepth = false;
    pipelineDesc.depthAndStencilDesc.enableStencil = false;

    renderPipelineState = gte.graphicsEngine->makeRenderPipelineState(pipelineDesc);
    MessageBoxA(GetForegroundWindow(),"App Pre Launch -- Stage 2","NOTE",MB_OK);


    MessageBoxA(GetForegroundWindow(),"App Started..","NOTE",MB_OK);
       
    renderTarget = gte.graphicsEngine->makeNativeRenderTarget(renderTargetDesc);

    tessContext = gte.tessalationEngine->createTEContextFromNativeRenderTarget(renderTarget);

    tessalate();

    MessageBoxA(GetForegroundWindow(),"Loaded Stage 1","NOTE",MB_OK);


    auto commandBuffer = renderTarget->commandBuffer();

    MessageBoxA(GetForegroundWindow(),"Loaded Stage 2","NOTE",MB_OK);
    OmegaGTE::GERenderTarget::RenderPassDesc renderPassDesc;
    using ColorAttachment = OmegaGTE::GERenderTarget::RenderPassDesc::ColorAttachment;
    renderPassDesc.colorAttachment = new ColorAttachment(ColorAttachment::ClearColor(1.f,1.f,0.f,1.f),ColorAttachment::Clear);

    OmegaGTE::GEViewport viewport {0,0,500 * scaleFactor,500 * scaleFactor,0,0};
    OmegaGTE::GEScissorRect scissorRect {0,0,500 * scaleFactor,500 * scaleFactor};

    commandBuffer->startRenderPass(renderPassDesc);
    commandBuffer->setRenderPipelineState(renderPipelineState);
    commandBuffer->setResourceConstAtVertexFunc(vertexBuffer,0);
    commandBuffer->setScissorRects({scissorRect});
    commandBuffer->setViewports({viewport});
    commandBuffer->drawPolygons(OmegaGTE::GERenderTarget::CommandBuffer::Triangle,6,0);
    MessageBoxA(GetForegroundWindow(),"Loaded Stage 3","NOTE",MB_OK);
    commandBuffer->endRenderPass();
     MessageBoxA(GetForegroundWindow(),"Loaded Stage 4","NOTE",MB_OK);

    renderTarget->submitCommandBuffer(commandBuffer);
    MessageBoxA(GetForegroundWindow(),"Loaded Stage 5","NOTE",MB_OK);

    renderTarget->commitAndPresent();
    MessageBoxA(GetForegroundWindow(),"Loaded Stage 6","NOTE",MB_OK);

    ShowWindow(hwnd,nShowCmd);
    

    UpdateWindow(hwnd);



    /// Message Loop
    MSG msg = {};
    
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


    OmegaGTE::Close(gte);

    return msg.wParam;
};



LRESULT CALLBACK   WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    LRESULT lr = 0;
    switch (uMsg) {
    case WM_PAINT : {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd,&ps);




        EndPaint(hwnd,&ps);
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProcA(hwnd,uMsg,wParam,lParam);
        break;
    }

    return lr;
};
