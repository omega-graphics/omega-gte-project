#include "omegaGTE/GE.h"
#include <windows.h>

OmegaGTE::SharedHandle<OmegaGTE::OmegaGraphicsEngine> graphicsEngine;

LRESULT CALLBACK   WndProc(HWND, UINT, WPARAM, LPARAM);

APIENTRY int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){

    graphicsEngine = OmegaGTE::OmegaGraphicsEngine::Create();

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


    HWND hwnd = CreateWindow("Test","",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,hInstance,NULL);
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
