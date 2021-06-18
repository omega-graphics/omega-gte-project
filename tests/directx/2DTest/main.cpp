#include <OmegaGTE.h>
#include <windows.h>


LRESULT CALLBACK   WndProc(HWND, UINT, WPARAM, LPARAM);

OmegaGTE::GTE gte;
static OmegaGTE::SharedHandle<OmegaGTE::GENativeRenderTarget> renderTarget;
static OmegaGTE::SharedHandle<OmegaGTE::OmegaTessalationEngineContext> tessContext;
static OmegaGTE::SharedHandle<OmegaGTE::GEBuffer> vertexBuffer;

void formatGPoint3D(std::ostream & os,OmegaGTE::GPoint3D & pt){
    os << "{ x:" << pt.x << ", y:" << pt.y << ", z:" << pt.z << "}";
};

void tessalate(){

    OmegaGTE::GRect rect;
    rect.h = 100;
    rect.w = 100;
    rect.pos.x = 0;
    rect.pos.y = 0;
    auto rect_mesh = tessContext->tessalateSync(OmegaGTE::TETessalationParams::Rect(rect));

    std::cout << "Tessalated GRect" << std::endl;
    OmegaGTE::FMatrix color = OmegaGTE::FMatrix::Color(1.f,0.f,0.f,1.f);
    std::cout << "Created Matrix GRect" << std::endl;
    OmegaGTE::ColoredVertexVector vertexVector;

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
            auto vertex = OmegaGTE::GEColoredVertex::FromGPoint3D(tri.a,color);
                std::cout << "Created Vertex 1" << std::endl;
            vertexVector.push_back(vertex);
            std::cout << "Pushed Vertex" << std::endl;
            vertexVector.push_back(OmegaGTE::GEColoredVertex::FromGPoint3D(tri.b,color));
            std::cout << "Created Vertex 2" << std::endl;
            vertexVector.push_back(OmegaGTE::GEColoredVertex::FromGPoint3D(tri.c,color));
            std::cout << "Created Vertex 3" << std::endl;
        };
    };

    vertexBuffer = tessContext->convertToVertexBuffer(gte.graphicsEngine,vertexVector);

};

APIENTRY int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){

    gte = OmegaGTE::Init();

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


    HWND hwnd = CreateWindowA(MAKEINTATOM(a),"",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,hInstance,NULL);
    if(!IsWindow(hwnd)){
         MessageBoxA(GetForegroundWindow(),"Failed to Create Window","NOTE",MB_OK);
        exit(1);
    };
    OmegaGTE::NativeRenderTargetDescriptor renderTargetDesc;

    renderTargetDesc.hwnd = hwnd;
    renderTargetDesc.isHwnd = true;




    MessageBoxA(GetForegroundWindow(),"App Started..","NOTE",MB_OK);
       
    renderTarget = gte.graphicsEngine->makeNativeRenderTarget(renderTargetDesc);

    tessContext = gte.tessalationEngine->createTEContextFromNativeRenderTarget(renderTarget);

    tessalate();

    MessageBoxA(GetForegroundWindow(),"Loaded Stage 1","NOTE",MB_OK);


    auto commandBuffer = renderTarget->commandBuffer();

    MessageBoxA(GetForegroundWindow(),"Loaded Stage 2","NOTE",MB_OK);
    OmegaGTE::GERenderTarget::RenderPassDesc renderPassDesc;
    using ColorAttachment = OmegaGTE::GERenderTarget::RenderPassDesc::ColorAttachment;
    renderPassDesc.colorAttachment = new ColorAttachment(ColorAttachment::ClearColor(1.f,0.f,0.f,1.f),ColorAttachment::Clear);

    commandBuffer->startRenderPass(renderPassDesc);
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
