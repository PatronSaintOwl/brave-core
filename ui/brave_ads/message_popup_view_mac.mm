/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "brave/ui/brave_ads/message_popup_view.h"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

#include "ui/views/widget/widget.h"

namespace gfx {
class Rect;
}  // namespace gfx

namespace brave_ads {

gfx::Rect MessagePopupView::GetVisibleFrameForPrimaryDisplay() const {
  NSRect frame = [[NSScreen mainScreen] frame];
  NSRect visibleFrame = [[NSScreen mainScreen] visibleFrame];
  // Normalize UI coordinate system origin from bottom-left to top-left
  return gfx::Rect(visibleFrame.origin.x, frame.size.height
      - visibleFrame.size.height, visibleFrame.size.width,
          visibleFrame.size.height);
}

float MessagePopupView::GetOpacity() const {
  if (!IsWidgetValid())
    return 0.f;
  return [GetWidget()->GetNativeWindow().GetNativeNSWindow() alphaValue];
}

}  // namespace brave_ads
