#include <OmegaGTE.h>

#import <Cocoa/Cocoa.h>
#include <sstream>
#import <QuartzCore/QuartzCore.h>
#include <iostream>

static OmegaGTE::GTE gte;
static OmegaGTE::SharedHandle<OmegaGTE::GTEShaderLibrary> funcLib;
static OmegaGTE::SharedHandle<OmegaGTE::GEBufferWriter> bufferWriter;
static OmegaGTE::SharedHandle<OmegaGTE::GERenderPipelineState> renderPipeline;
static OmegaGTE::SharedHandle<OmegaGTE::GENativeRenderTarget> nativeRenderTarget = nullptr;
static OmegaGTE::SharedHandle<OmegaGTE::OmegaTessalationEngineContext> tessContext;

// static OmegaGTE::SharedHandle<OmegaGTE::GEFunction> vertexFunc;
// static OmegaGTE::SharedHandle<OmegaGTE::GEFunction> fragmentFunc;
// static OmegaGTE::SharedHandle<OmegaGTE::GERenderTarget::CommandBuffer> commandBuffer;

void formatGPoint3D(std::ostream & os,OmegaGTE::GPoint3D & pt){
    os << "{ x:" << pt.x << ", y:" << pt.y << ", z:" << pt.z << "}";
};

static void writeVertex(OmegaGTE::GPoint3D & pt,OmegaGTE::FVec<4> &color){
    auto pos_vec = OmegaGTE::FVec<4>::Create();
    pos_vec[0][0] = pt.x;
    pos_vec[1][0] = pt.y;
    pos_vec[2][0] = pt.z;
    pos_vec[3][0] = 0.f;

    bufferWriter->structBegin();
    bufferWriter->writeFloat4(pos_vec);
    bufferWriter->writeFloat4(color);
    bufferWriter->structEnd();
}

static void render(){



    OmegaGTE::GRect rect;
    rect.h = 100;
    rect.w = 100;
    rect.pos.x = 0;
    rect.pos.y = 0;
    auto rect_mesh = tessContext->tessalateSync(OmegaGTE::TETessalationParams::Rect(rect));


    std::cout << "Tessalated GRect" << std::endl;
    auto color = OmegaGTE::makeColor(1.f,0.f,0.f,1.f);
    std::cout << "Created Matrix GRect" << std::endl;

    OmegaGTE::BufferDescriptor bufferDescriptor {6 * (FLOAT4_SIZE + FLOAT4_SIZE),(FLOAT4_SIZE + FLOAT4_SIZE)};
    auto vertexBuffer = gte.graphicsEngine->makeBuffer(bufferDescriptor);

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
            writeVertex(tri.a,color);
            writeVertex(tri.b,color);
            writeVertex(tri.c,color);
        };
    };

    bufferWriter->finish();



        

    auto commandBuffer = nativeRenderTarget->commandBuffer();
    NSLog(@"Command Buffer Created");
    OmegaGTE::GERenderTarget::RenderPassDesc renderPass;
    using RenderPassDesc = OmegaGTE::GERenderTarget::RenderPassDesc;
    renderPass.colorAttachment = new RenderPassDesc::ColorAttachment(RenderPassDesc::ColorAttachment::ClearColor(1.f,1.f,1.f,1.f),RenderPassDesc::ColorAttachment::Clear);
    NSLog(@"Starting Render Pass");
    commandBuffer->startRenderPass(renderPass);
    commandBuffer->setRenderPipelineState(renderPipeline);
    commandBuffer->setResourceConstAtVertexFunc(vertexBuffer,0);
    commandBuffer->drawPolygons(OmegaGTE::GERenderTarget::CommandBuffer::Triangle,vertexBuffer->size(),0);
    NSLog(@"Ending Render Pass");
    commandBuffer->endRenderPass();
    NSLog(@"Ended Render Pass");
    nativeRenderTarget->submitCommandBuffer(commandBuffer);
    NSLog(@"Command Buffer Scheduled for Execution");

    nativeRenderTarget->commitAndPresent();
    NSLog(@"Presenting Frame"); 


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
        
        render();
    
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

#define VERTEX_FUNC "vertexFunc"
#define FRAGMENT_FUNC "fragFunc"


int main(int argc,const char * argv[]){
    
    gte = OmegaGTE::Init();

   funcLib = gte.graphicsEngine->loadShaderLibrary("./shaders.omegasllib");

   bufferWriter = OmegaGTE::GEBufferWriter::Create();

   std::cout << "LIBRARY SIZE:" << funcLib->shaders.size() << std::endl;

   OmegaGTE::RenderPipelineDescriptor pipelineDesc;
   pipelineDesc.vertexFunc = funcLib->shaders[VERTEX_FUNC];
   pipelineDesc.fragmentFunc = funcLib->shaders[FRAGMENT_FUNC];
   renderPipeline = gte.graphicsEngine->makeRenderPipelineState(pipelineDesc);


    return NSApplicationMain(argc,argv);
};
