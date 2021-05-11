#include <OmegaGTE.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

static OmegaGTE::GTE gte;

@interface MyWindowController : NSWindowController<NSWindowDelegate>
@end

@implementation MyWindowController

- (instancetype)init
{
    if(self = [super initWithWindow:[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,500,500) styleMask:NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO]]){
        self.window.delegate = self;

        NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0,0,200,200)];
        view.wantsLayer = TRUE;
        CAMetalLayer * metalLayer = [CAMetalLayer layer];
        view.layer = metalLayer;

    
        OmegaGTE::NativeRenderTargetDescriptor desc;
        desc.metalLayer = metalLayer;

        auto renderTarget = gte.graphicsEngine->makeNativeRenderTarget(desc);
        // auto commandBuffer = renderTarget->commandBuffer();
        // OmegaGTE::GERenderTarget::RenderPassDesc renderPass;
        // renderPass.
        // commandBuffer->startRenderPass()


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

    return NSApplicationMain(argc,argv);
};
