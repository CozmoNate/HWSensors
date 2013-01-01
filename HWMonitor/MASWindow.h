//
//  MASWindow.h
//  A Mac App Store-like NSWindow subclass.
//
//  Created by Zachary Waldowski on 1/8/11.
//  Secured under the "do-whatever-the-hell-you-want-with-it" license.  Erm, copyright 2011.
//

#import <Cocoa/Cocoa.h>

@interface MASWindow : NSWindow;

@property (nonatomic, readonly) CGFloat toolbarHeight;

-(void)_addResponders;
-(void)_removeResponders;
-(void)_repositionTrafficLights;
-(void)_resizeContentView;
-(void)_moveToolbar;

@end
