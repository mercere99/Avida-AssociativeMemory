//
//  MapGridView.m
//  avida/apps/viewer-macos
//
//  Created by David on 11/23/10.
//  Copyright 2010-2011 Michigan State University. All rights reserved.
//  http://avida.devosoft.org/viewer-macos
//
//  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
//  following conditions are met:
//  
//  1.  Redistributions of source code must retain the above copyright notice, this list of conditions and the
//      following disclaimer.
//  2.  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
//      following disclaimer in the documentation and/or other materials provided with the distribution.
//  3.  Neither the name of Michigan State University, nor the names of contributors may be used to endorse or promote
//      products derived from this software without specific prior written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY MICHIGAN STATE UNIVERSITY AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL MICHIGAN STATE UNIVERSITY OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
//  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  Authors: David M. Bryson <david@programerror.com>
//

#import "MapGridView.h"

#include "avida/viewer-core/Map.h"

#include <cassert>
#include <iostream>


static inline CGFloat sigmoid(CGFloat x, CGFloat midpoint, CGFloat steepness)
{
  CGFloat val = steepness * (x - midpoint);
  return exp(val) / (1.0 + exp(val));
}


@interface MapGridView (hidden) {  
}
- (void) setup;
@end

@implementation MapGridView (hidden)
- (void) setup {
  map_width = 0;
  map_height = 0;
  num_colors = 0;
  color_cache = [NSMutableArray arrayWithCapacity:255];
  zoom = -1;
  selected_x = -1;
  selected_y = -1;
  [self setWantsLayer:YES];  
}
@end



@implementation MapGridView

- (id) initWithFrame:(NSRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
    [self setup];
  }
  return self;
}


- (void) awakeFromNib {
  [self setup];
}


- (void) drawRect:(NSRect)dirtyRect {
  [[NSColor darkGrayColor] set];
  [NSBezierPath fillRect:dirtyRect];
  
  if (num_colors != [color_cache count]) {
    [color_cache removeAllObjects];
    if (num_colors == 10) {
      [color_cache insertObject:[NSColor greenColor] atIndex:0];
      [color_cache insertObject:[NSColor redColor] atIndex:1];
      [color_cache insertObject:[NSColor blueColor] atIndex:2];
      [color_cache insertObject:[NSColor cyanColor] atIndex:3];
      [color_cache insertObject:[NSColor yellowColor] atIndex:4];
      [color_cache insertObject:[NSColor magentaColor] atIndex:5];
      [color_cache insertObject:[NSColor orangeColor] atIndex:6];
      [color_cache insertObject:[NSColor purpleColor] atIndex:7];
      [color_cache insertObject:[NSColor brownColor] atIndex:8];
      [color_cache insertObject:[NSColor lightGrayColor] atIndex:9];
    } else {
      for (int i = 0; i < num_colors; i++) {
        CGFloat x = 0.1 + 0.8 * (static_cast<CGFloat>(i) / (num_colors - 1));
        CGFloat h = fmod((x + .27), 1.0);
        CGFloat s = sigmoid(1.0 - x, 0.1, 30);
        CGFloat b = sigmoid(x, 0.3, 10);
        [color_cache insertObject:[NSColor colorWithCalibratedHue:h saturation:s brightness:b alpha:1.0] atIndex:i];
      }
    }
  }
  
  CGFloat block_size = zoom;
  CGFloat grid_width = (block_size > 5.0) ? 1.0 : 0.0;
  
  // Determine Map Dimensions
  NSRect mapRect;
  mapRect.size.width = map_width * block_size - grid_width;
  mapRect.size.height = map_height * block_size - grid_width;  
  mapRect.origin = NSMakePoint(0, 0);
  
  [[NSColor blackColor] set];
  [NSBezierPath fillRect:mapRect];
  
  
  NSRect gridCellRect;
  gridCellRect.size.width = block_size - grid_width;
  gridCellRect.size.height = block_size - grid_width;

  for (int i = 0; i < map_width; i++) {
    for (int j = 0; j < map_height; j++) {
      gridCellRect.origin = NSMakePoint(mapRect.origin.x + block_size * i, mapRect.origin.y + block_size * j);
      int color = map_colors[i * map_width + j];
      switch (color) {
        case -4:  break;
        case -3:  [[NSColor darkGrayColor] set]; [NSBezierPath fillRect:gridCellRect]; break;
        case -2:  [[NSColor grayColor] set]; [NSBezierPath fillRect:gridCellRect]; break;
        case -1:  [[NSColor whiteColor] set]; [NSBezierPath fillRect:gridCellRect];break;
        default:  [(NSColor*)[color_cache objectAtIndex:color] set]; [NSBezierPath fillRect:gridCellRect]; break;
      }
      
      if (i == selected_x && j == selected_y) {
        // Handle selected cell preferentially to tags
        [[NSColor greenColor] set];
      } else {
        // Handle tag coloration
        int tag = map_tags[i * map_width + j];
        switch (tag) {
          case -4:  continue;
          case -3:  [[NSColor darkGrayColor] set]; break;
          case -2:  [[NSColor grayColor] set]; break;
          case -1:  [[NSColor whiteColor] set]; break;
          default:  continue;
        }
      }
      // Draw tag outline
      [NSGraphicsContext saveGraphicsState];
      NSBezierPath* tagPath = [NSBezierPath bezierPathWithRect:gridCellRect];
      [tagPath setLineWidth:2.0];
      [tagPath setLineCapStyle:NSSquareLineCapStyle];
      [tagPath setClip];
      [tagPath stroke];
      [NSGraphicsContext restoreGraphicsState];
    }
  }
}


- (BOOL) isOpaque {
  return YES;
}


- (void) updateState:(Avida::CoreView::Map*)state {
  state->Retain();
  
  map_width = state->GetWidth();
  map_height = state->GetHeight();
  
  map_colors = state->GetColors();
  num_colors = state->GetColorScale().GetScaleRange();
  
  map_tags = state->GetTags();

  state->Release();
  
  
  if (zoom < 0) {
    NSScrollView* scrollView = [self enclosingScrollView];
    assert(scrollView != nil);
    
    NSSize bounds = [scrollView bounds].size;
    double z1 = bounds.width / map_width;
    double z2 = bounds.height / map_height;
    double zval = (z1 > z2) ? z2 : z1;
    if (zval > 15.0) zval = 15.0;
    zval = floor(zval);
    [self setZoom:zval];
  } else {
    [self setNeedsDisplay:YES];
  }
}


- (void) mouseDown:(NSEvent*)event {
  
  if (selectionDelegate != nil) {
    // convert the mouse-down location into the view coords
    NSPoint clickLocation = [self convertPoint:[event locationInWindow] fromView:nil];
    
    CGFloat block_size = zoom;
    
    NSPoint selectedOrg;
    selectedOrg.x = floor(clickLocation.x / block_size);
    selectedOrg.y = floor(clickLocation.y / block_size);
    if (selected_x != selectedOrg.x || selected_y != selectedOrg.y) {
      if (selectionDelegate == nil ||
          ![selectionDelegate respondsToSelector:@selector(mapView:shouldSelectObjectAtPoint:)] ||
          [selectionDelegate mapView:self shouldSelectObjectAtPoint:selectedOrg]) {
        selected_x = selectedOrg.x;
        selected_y = selectedOrg.y;
        [selectionDelegate mapViewSelectionChanged:self];
      }
    }
  }
  
}

- (NSColor*) colorOfX:(int)x Y:(int)y {
  int color = map_colors[x * map_width + y];
  switch (color) {
    case -4:  return [NSColor blackColor];
    case -3:  return [NSColor darkGrayColor];
    case -2:  return [NSColor grayColor];
    case -1:  return [NSColor whiteColor];
    default:  return (NSColor*)[color_cache objectAtIndex:color];
  }
  return [NSColor blackColor];
}

@synthesize zoom;
- (void) setZoom:(double)zval {
  zoom = round(zval);
  
  CGFloat block_size = zoom;
  CGFloat grid_width = (block_size > 5.0) ? 1.0 : 0.0;

  NSSize mapSize;
  mapSize.width = map_width * block_size - grid_width;
  mapSize.height = map_height * block_size - grid_width;
  
  [self setFrameSize:mapSize];
  
  [self setNeedsDisplay:YES];
}

@synthesize selectionDelegate;


- (NSPoint) selectedObject {
  return NSMakePoint(selected_x, selected_y);
}

- (void) setSelectedObject:(NSPoint)point {
  int new_x = floor(point.x);
  int new_y = floor(point.y);
  if (new_x < map_width && new_x >= 0 && new_y < map_height && new_y >= 0) {
    if (new_x != selected_x || new_y != selected_y) {
      selected_x = floor(point.x);
      selected_y = floor(point.y);
      [selectionDelegate mapViewSelectionChanged:self];
    }
  }
}

- (void) clearSelectedObject {
  if (selected_x != -1) {
    selected_x = -1;
    selected_y = -1;
    [selectionDelegate mapViewSelectionChanged:self];
  }
}


- (void) setFrame:(NSRect)frameRect {
  [super setFrame:frameRect];
}

- (void) setFrameOrigin:(NSPoint)newOrigin {
  [super setFrameOrigin:newOrigin];
}

- (void) setFrameSize:(NSSize)newSize {
  [super setFrameSize:newSize];
}

- (void) setFrameRotation:(CGFloat)angle {
  [super setFrameRotation:angle];
}


@end
