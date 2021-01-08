/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/sidebar/sidebar_add_item_bubble_delegate_view.h"

#include <memory>

#include "base/strings/utf_string_conversions.h"
#include "brave/browser/themes/theme_properties.h"
#include "brave/browser/ui/brave_browser.h"
#include "brave/browser/ui/sidebar/sidebar_controller.h"
#include "brave/browser/ui/sidebar/sidebar_service_factory.h"
#include "brave/browser/ui/sidebar/sidebar_utils.h"
#include "brave/components/sidebar/sidebar_service.h"
#include "brave/grit/brave_generated_resources.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/theme_provider.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/separator.h"
#include "ui/views/layout/box_layout.h"
#include "url/gurl.h"

namespace {

sidebar::SidebarService* GetSidebarService(Browser* browser) {
  return sidebar::SidebarServiceFactory::GetForProfile(browser->profile());
}

}  // namespace

SidebarAddItemBubbleDelegateView::SidebarAddItemBubbleDelegateView(
    BraveBrowser* browser,
    views::View* anchor_view)
    : BubbleDialogDelegateView(anchor_view, views::BubbleBorder::LEFT_TOP),
      browser_(browser) {
  DCHECK(browser_);
  SetButtons(ui::DIALOG_BUTTON_NONE);
  SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical));
  AddChildViews();
}

SidebarAddItemBubbleDelegateView::~SidebarAddItemBubbleDelegateView() = default;

void SidebarAddItemBubbleDelegateView::AddChildViews() {
  // Default font list is usually 12pt. Increase 1pt to make it 13px.
  views::Label::CustomFont font = {
      views::Label::GetDefaultFontList()
          .DeriveWithSizeDelta(1)
          .DeriveWithWeight(gfx::Font::Weight::SEMIBOLD)};
  AddChildView(std::make_unique<views::Label>(
      l10n_util::GetStringUTF16(IDS_SIDEBAR_ADD_ITEM_BUBBLE_TITLE), font));

  if (sidebar::CanAddCurrentActiveTabToSidebar(browser_)) {
    auto* button = AddChildView(std::make_unique<views::LabelButton>());
    const GURL active_tab_url =
        browser_->tab_strip_model()->GetActiveWebContents()->GetVisibleURL();
    button->SetText(base::UTF8ToUTF16(active_tab_url.host()));
    button->SetCallback(base::BindRepeating(
        &SidebarAddItemBubbleDelegateView::OnCurrentItemButtonPressed,
        base::Unretained(this)));
  }

  const auto not_added_default_items =
      GetSidebarService(browser_)->GetNotAddedDefaultSidebarItems();
  if (not_added_default_items.empty())
    return;

  auto* separator = AddChildView(std::make_unique<views::Separator>());
  if (const ui::ThemeProvider* theme_provider = GetThemeProvider()) {
    separator->SetColor(theme_provider->GetColor(
        BraveThemeProperties::COLOR_SIDEBAR_SEPARATOR));
  }

  for (const auto& item : not_added_default_items) {
    auto* button = AddChildView(std::make_unique<views::LabelButton>());
    button->SetText(item.title);
    button->SetCallback(base::BindRepeating(
        &SidebarAddItemBubbleDelegateView::OnDefaultItemsButtonPressed,
        base::Unretained(this), item));
  }
}

void SidebarAddItemBubbleDelegateView::OnDefaultItemsButtonPressed(
    const sidebar::SidebarItem& item) {
  GetSidebarService(browser_)->AddItem(item);
  RemoveAllChildViews(true);
  AddChildViews();
  if (children().size() == 1)
    GetWidget()->CloseWithReason(views::Widget::ClosedReason::kUnspecified);
}

void SidebarAddItemBubbleDelegateView::OnCurrentItemButtonPressed() {
  browser_->sidebar_controller()->AddItemWithCurrentTab();
  RemoveAllChildViews(true);
  AddChildViews();
  if (children().size() == 1)
    GetWidget()->CloseWithReason(views::Widget::ClosedReason::kUnspecified);
}
