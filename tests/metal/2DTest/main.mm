#include <OmegaGTE.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#include <iostream>

static OmegaGTE::GTE gte;
static OmegaGTE::SharedHandle<OmegaGTE::GEFunctionLibrary> funcLib;
static OmegaGTE::SharedHandle<OmegaGTE::GERenderPipelineState> renderPipeline;
static OmegaGTE::SharedHandle<OmegaGTE::GENativeRenderTarget> nativeRenderTarget = nullptr;
//static OmegaGTE::SharedHandle<OmegaGTE::GERenderTarget::CommandBuffer> commandBuffer;

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

        auto tessContext = gte.tessalationEngine->createTEContextFromNativeRenderTarget(nativeRenderTarget);
        
        OmegaGTE::GRect rect;
        auto rect_mesh = tessContext->tessalateSync(OmegaGTE::TETessalationParams::Rect(rect));

        

        auto commandBuffer = nativeRenderTarget->commandBuffer();
        NSLog(@"Command Buffer Created");
        OmegaGTE::GERenderTarget::RenderPassDesc renderPass;
        using RenderPassDesc = OmegaGTE::GERenderTarget::RenderPassDesc;
        renderPass.colorAttachment = new RenderPassDesc::ColorAttachment(RenderPassDesc::ColorAttachment::ClearColor(1.f,0.f,0.f,1.f),RenderPassDesc::ColorAttachment::Clear);
        NSLog(@"Starting Render Pass");
        commandBuffer->startRenderPass(renderPass);
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

    // NSString * mainBundleUrl = [[NSBundle mainBundle] pathForResource:@"MyShaderLib" ofType:@"metallib"];

    // std::filesystem::path p(mainBundleUrl.UTF8String);

    // funcLib = gte.graphicsEngine->loadLibrary(p,{"myVertexFunc","myFragmentFunc"});

    // OmegaGTE::RenderPipelineDescriptor pipelineDesc;
    // pipelineDesc.vertexFunc = nullptr;
    // pipelineDesc.fragmentFunc = nullptr;

    // renderPipeline = gte.graphicsEngine->makeRenderPipelineState(pipelineDesc);

    return NSApplicationMain(argc,argv);
};
