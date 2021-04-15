#include <OmegaGTE.h>

#import <Cocoa/Cocoa.h>

OmegaGTE::SharedHandle<OmegaGTE::OmegaGraphicsEngine> graphicsEngine;

@interface MyWindowController : NSWindowController<NSWindowDelegate>
@end

@implementation MyWindowController

- (instancetype)init
{
    if(self = [super initWithWindow:[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,500,500) styleMask:NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO]]){
        self.window.delegate = self;
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
    
};

@end



int main(int argc,const char * argv[]){
    graphicsEngine = OmegaGTE::OmegaGraphicsEngine::Create();
    return NSApplicationMain(argc,argv);
};
