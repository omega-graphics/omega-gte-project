#include <OmegaGTE.h>

#import <Cocoa/Cocoa.h>
#include <sstream>
#import <QuartzCore/QuartzCore.h>
#include <iostream>

static OmegaGTE::GTE gte;
static OmegaGTE::SharedHandle<OmegaGTE::GEFunctionLibrary> funcLib;
static OmegaGTE::SharedHandle<OmegaGTE::GERenderPipelineState> renderPipeline;
static OmegaGTE::SharedHandle<OmegaGTE::GENativeRenderTarget> nativeRenderTarget = nullptr;
static OmegaGTE::SharedHandle<OmegaGTE::OmegaTessalationEngineContext> tessContext;

static OmegaGTE::SharedHandle<OmegaGTE::GEFunction> vertexFunc;
static OmegaGTE::SharedHandle<OmegaGTE::GEFunction> fragmentFunc;
// static OmegaGTE::SharedHandle<OmegaGTE::GERenderTarget::CommandBuffer> commandBuffer;

void formatGPoint3D(std::ostream & os,OmegaGTE::GPoint3D & pt){
    os << "{ x:" << pt.x << ", y:" << pt.y << ", z:" << pt.z << "}";
};
@interface MyWindowController : NSWindowController<NSWindowDelegate>
@end

@implementation MyWindowController

- (instancetype)init
{
    if(self = [super initWithWindow:[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,500,500) styleMask:NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO]]){
        self.window.delegate = self;
        NSView *rootView = [[NSView alloc] initWithFrame:NSZeroRect];
        NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0,0,200,200)];
        view.wantsLayer = TRUE;
        CAMetalLayer * metalLayer = [CAMetalLayer layer];
        metalLayer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
        metalLayer.frame = view.frame;
        // CALayer *regLayer = [CALayer layer];
        // regLayer.backgroundColor = [NSColor blueColor].CGColor;
        view.layer = metalLayer;
        // view.layer = regLayer;


    
        OmegaGTE::NativeRenderTargetDescriptor desc;
        desc.metalLayer = metalLayer;

        nativeRenderTarget = gte.graphicsEngine->makeNativeRenderTarget(desc);

        tessContext = gte.tessalationEngine->createTEContextFromNativeRenderTarget(nativeRenderTarget);
        
        OmegaGTE::GRect rect;
        rect.h = 100;
        rect.w = 100;
        rect.pos.x = 0;
        rect.pos.y = 0;
        auto rect_mesh = tessContext->tessalateSync(OmegaGTE::TETessalationParams::Rect(rect));

        OmegaGTE::FMatrix color = OmegaGTE::FMatrix::Color(1.f,0.f,0.f,1.f);

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
                auto vertex = OmegaGTE::GEColoredVertex::FromGPoint3D(tri.a,color);
                vertexVector.push_back(vertex);
                vertexVector.push_back(OmegaGTE::GEColoredVertex::FromGPoint3D(tri.b,color));
                vertexVector.push_back(OmegaGTE::GEColoredVertex::FromGPoint3D(tri.c,color));
            };
        };

        auto vertexBuffer = tessContext->convertToVertexBuffer(gte.graphicsEngine,vertexVector);
        

        auto commandBuffer = nativeRenderTarget->commandBuffer();
        NSLog(@"Command Buffer Created");
        OmegaGTE::GERenderTarget::RenderPassDesc renderPass;
        using RenderPassDesc = OmegaGTE::GERenderTarget::RenderPassDesc;
        renderPass.colorAttachment = new RenderPassDesc::ColorAttachment(RenderPassDesc::ColorAttachment::ClearColor(1.f,1.f,1.f,1.f),RenderPassDesc::ColorAttachment::Clear);
        NSLog(@"Starting Render Pass");
        commandBuffer->startRenderPass(renderPass);
        commandBuffer->setRenderPipelineState(renderPipeline);
        commandBuffer->setResourceConstAtVertexFunc(vertexBuffer,0);
        NSLog(@"Ending Render Pass");
        commandBuffer->endRenderPass();
        NSLog(@"Ended Render Pass");
        nativeRenderTarget->submitCommandBuffer(commandBuffer);
        NSLog(@"Command Buffer Scheduled for Execution");

        nativeRenderTarget->commitAndPresent();
        NSLog(@"Presenting Frame"); 
        [rootView addSubview:view];

        [self.window setContentView:rootView];
        [self.window center];
        [self.window layoutIfNeeded];
    }
    return self;
}
- (void)windowWillClose:(NSNotification *)notification{
    [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0];
};

@end

@interface AppDelegate : NSObject<NSApplicationDelegate>
//@property(nonatomic,retain) NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification{
    NSLog(@"App Finished Launching");
    auto windowController = [[MyWindowController alloc] init];
    [windowController showWindow:self];
};

- (void)applicationWillTerminate:(NSNotification *)notification {
    OmegaGTE::Close(gte);
};

@end



int main(int argc,const char * argv[]){
    
    gte = OmegaGTE::Init();

    funcLib = gte.graphicsEngine->loadStdShaderLibrary();

    std::cout << "LIBRARY SIZE:" << funcLib->functions.size() << std::endl;

    OmegaGTE::RenderPipelineDescriptor pipelineDesc;
    vertexFunc = funcLib->functions[STD_COLOREDVERTEX_FUNC];
    fragmentFunc = funcLib->functions[STD_FRAGMENTVERTEX_FUNC];
    pipelineDesc.vertexFunc = vertexFunc;
    pipelineDesc.fragmentFunc = fragmentFunc;
    renderPipeline = gte.graphicsEngine->makeRenderPipelineState(pipelineDesc);


    return NSApplicationMain(argc,argv);
};
