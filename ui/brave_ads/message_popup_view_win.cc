/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "brave/ui/brave_ads/message_popup_view.h"

#include "ui/gfx/win/window_impl.h"

namespace gfx {
class Rect;
}  // namespace gfx

namespace brave_ads {

gfx::Rect MessagePopupView::GetVisibleFrameForPrimaryDisplay() const {
  RECT rect;
  GetClientRect(GetDesktopWindow(), &rect);
  return gfx::Rect(rect);
}

}  // namespace brave_ads
