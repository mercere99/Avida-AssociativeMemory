//
//  MapGridView.h
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

#import <Cocoa/Cocoa.h>

#include "apto/core.h"

namespace Avida {
  namespace CoreView {
    class Map;
  };
};

@class MapGridView;

@protocol MapSelectionDelegate <NSObject>
@optional
- (BOOL) mapView:(MapGridView*)mapView shouldSelectObjectAtPoint:(NSPoint)point;
- (void) mapViewSelectionChanged:(MapGridView*)mapView;
@end


@interface MapGridView : NSView {
  int map_width;
  int map_height;
  int num_colors;
  double zoom;
  
  Apto::Array<int> map_colors;
  Apto::Array<int> map_tags;
  NSMutableArray* color_cache;
  
  IBOutlet id<MapSelectionDelegate> selectionDelegate;
  int selected_x;
  int selected_y;
}

- (id) initWithFrame:(NSRect)frame;
- (void) awakeFromNib;

- (void) drawRect:(NSRect)rect;
- (BOOL) isOpaque;

- (void) updateState:(Avida::CoreView::Map*)state;

- (void) mouseDown:(NSEvent*)event;

- (NSColor*) colorOfX:(int)x Y:(int)y;

@property (readwrite, nonatomic) double zoom;
@property (readwrite, retain) id<MapSelectionDelegate> selectionDelegate;

@property (readwrite, nonatomic) NSPoint selectedObject;
- (void) clearSelectedObject;

@end
