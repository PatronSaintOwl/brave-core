/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/sidebar/sidebar_items_container_view.h"

#include "base/bind.h"
#include "base/notreached.h"
#include "base/strings/utf_string_conversions.h"
#include "brave/app/vector_icons/vector_icons.h"
#include "brave/browser/themes/theme_properties.h"
#include "brave/browser/ui/brave_browser.h"
#include "brave/browser/ui/sidebar/sidebar_controller.h"
#include "brave/browser/ui/views/sidebar/sidebar_item_view.h"
#include "brave/components/sidebar/sidebar_item.h"
#include "brave/grit/brave_generated_resources.h"
#include "brave/grit/brave_theme_resources.h"
#include "chrome/browser/favicon/favicon_service_factory.h"
#include "components/favicon/core/favicon_service.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/theme_provider.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/layout/box_layout.h"

SidebarItemsContainerView::SidebarItemsContainerView(BraveBrowser* browser)
    : browser_(browser),
      sidebar_model_(browser->sidebar_controller()->model()),
      task_tracker_(new base::CancelableTaskTracker{}) {
  DCHECK(browser_);
  set_context_menu_controller(this);
  SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical));

  DCHECK(browser->sidebar_controller());
  observed_.Add(sidebar_model_);
}

SidebarItemsContainerView::~SidebarItemsContainerView() = default;

gfx::Size SidebarItemsContainerView::CalculatePreferredSize() const {
  if (children().empty())
    return {0, 0};
  const gfx::Size child_size = children()[0]->GetPreferredSize();
  return {child_size.width() + GetInsets().width(),
          children().size() * child_size.height() + GetInsets().height()};
}

void SidebarItemsContainerView::OnThemeChanged() {
  View::OnThemeChanged();

  UpdateAllBuiltInItemsViewState();
}

void SidebarItemsContainerView::UpdateAllBuiltInItemsViewState() {
  const auto items = sidebar_model_->GetAllSidebarItems();
  // It's not initialized yet if child view count and items size are different.
  if (children().size() != items.size())
    return;

  // BuiltIn items different colored images depends on theme.
  const int active_index = sidebar_model_->active_index();
  int index = 0;
  for (const auto& item : items) {
    if (sidebar::IsBuiltInType(item))
      UpdateItemViewStateAt(index, index == active_index);
    index++;
  }
}

base::string16 SidebarItemsContainerView::GetTooltipTextFor(
    const views::View* view) const {
  int index = GetIndexOf(view);
  DCHECK_GE(index, 0);

  if (index == -1)
    return base::string16();

  auto item = sidebar_model_->GetAllSidebarItems()[index];
  if (!item.title.empty())
    return item.title;

  return base::UTF8ToUTF16(item.url.spec());
}

void SidebarItemsContainerView::ShowContextMenuForViewImpl(
    views::View* source,
    const gfx::Point& point,
    ui::MenuSourceType source_type) {
  if (context_menu_runner_ && context_menu_runner_->IsRunning())
    return;

  if (GetIndexOf(source) == -1)
    return;

  view_for_context_menu_ = source;
  context_menu_model_ = std::make_unique<ui::SimpleMenuModel>(this);
  context_menu_model_->AddItem(
      kItemRemove,
      l10n_util::GetStringUTF16(IDS_SIDEBAR_ITEM_CONTEXT_MENU_REMOVE));
  context_menu_runner_ = std::make_unique<views::MenuRunner>(
      context_menu_model_.get(), views::MenuRunner::CONTEXT_MENU,
      base::BindRepeating(&SidebarItemsContainerView::OnContextMenuClosed,
                          base::Unretained(this)));
  context_menu_runner_->RunMenuAt(
      source->GetWidget(), nullptr, gfx::Rect(point, gfx::Size()),
      views::MenuAnchorPosition::kTopLeft, source_type);
}

void SidebarItemsContainerView::ExecuteCommand(int command_id,
                                               int event_flags) {
  int index = GetIndexOf(view_for_context_menu_);
  DCHECK_GE(index, 0);

  if (index == -1)
    return;

  if (command_id == kItemRemove) {
    browser_->sidebar_controller()->RemoveItemAt(index);
    return;
  }
}

void SidebarItemsContainerView::OnContextMenuClosed() {
  view_for_context_menu_ = nullptr;
}

void SidebarItemsContainerView::OnItemAdded(const sidebar::SidebarItem& item,
                                            int index) {
  AddItemView(item, index);
}

void SidebarItemsContainerView::OnItemRemoved(int index) {
  RemoveChildView(children()[index]);
  InvalidateLayout();
  Layout();
}

void SidebarItemsContainerView::OnActiveIndexChanged(int old_index,
                                                     int new_index) {
  if (old_index != -1)
    UpdateItemViewStateAt(old_index, false);

  if (new_index != -1)
    UpdateItemViewStateAt(new_index, true);
}

void SidebarItemsContainerView::AddItemView(const sidebar::SidebarItem& item,
                                            int index) {
  auto* item_view =
      AddChildViewAt(std::make_unique<SidebarItemView>(this), index);
  item_view->set_context_menu_controller(this);
  item_view->set_paint_background_on_hovered(sidebar::IsWebType(item));
  item_view->SetCallback(
      base::BindRepeating(&SidebarItemsContainerView::OnItemPressed,
                          base::Unretained(this), item_view));

  UpdateItemViewStateAt(index, false);
  // Web type uses site favicon as button's image.
  if (sidebar::IsWebType(item))
    FetchFavicon(item, index);

  InvalidateLayout();
  Layout();
}

bool SidebarItemsContainerView::IsBuiltInTypeItemView(views::View* view) const {
  const int index = GetIndexOf(view);
  return sidebar::IsBuiltInType(sidebar_model_->GetAllSidebarItems()[index]);
}

void SidebarItemsContainerView::FetchFavicon(const sidebar::SidebarItem& item,
                                             int index) {
  // Use favicon as a web type icon's image.
  auto* favicon_service = FaviconServiceFactory::GetForProfile(
      browser_->profile(), ServiceAccessType::EXPLICIT_ACCESS);
  favicon_service->GetFaviconImageForPageURL(
      item.url,
      base::BindRepeating(&SidebarItemsContainerView::OnGetFaviconImage,
                          weak_ptr_factory_.GetWeakPtr(), item),
      task_tracker_.get());
}

void SidebarItemsContainerView::OnGetFaviconImage(
    const sidebar::SidebarItem& item,
    const favicon_base::FaviconImageResult& image_result) {
  // TODO(simonhong): Use default image if we can't get site favicon.
  // If history is cleared, we can't get existing item's favicon.
  if (image_result.image.IsEmpty())
    return;

  int index = sidebar_model_->GetIndexOf(item);
  // -1 means |item| is deleted while fetching favicon.
  if (index == -1)
    return;

  SidebarItemView* item_view = static_cast<SidebarItemView*>(children()[index]);
  item_view->SetImage(views::Button::STATE_NORMAL,
                      image_result.image.AsImageSkia());
}

void SidebarItemsContainerView::UpdateItemViewStateAt(int index, bool active) {
  const auto item = sidebar_model_->GetAllSidebarItems()[index];
  SidebarItemView* item_view = static_cast<SidebarItemView*>(children()[index]);

  if (item.open_in_panel)
    item_view->set_draw_highlight(active);

  if (sidebar::IsBuiltInType(item)) {
    const GURL& url = item.url;
    item_view->SetImage(views::Button::STATE_NORMAL,
                        GetImageForBuiltInItems(url, active));
    item_view->SetImage(views::Button::STATE_HOVERED,
                        GetImageForBuiltInItems(url, true));
    item_view->SetImage(views::Button::STATE_PRESSED,
                        GetImageForBuiltInItems(url, true));
  }
}

void SidebarItemsContainerView::OnItemPressed(const views::View* item) {
  auto* controller = browser_->sidebar_controller();
  if (controller->IsActiveIndex(GetIndexOf(item))) {
    // TODO(simonhong): This is for demo. We will have another UI for closing.
    // De-activate active item.
    controller->ActivateItemAt(-1);
    return;
  }

  controller->ActivateItemAt(GetIndexOf(item));
}

gfx::ImageSkia SidebarItemsContainerView::GetImageForBuiltInItems(
    const GURL& item_url,
    bool focused) const {
  SkColor base_button_color = SK_ColorWHITE;
  if (const ui::ThemeProvider* theme_provider = GetThemeProvider()) {
    base_button_color = theme_provider->GetColor(
        BraveThemeProperties::COLOR_SIDEBAR_BUTTON_BASE);
  }
  auto& bundle = ui::ResourceBundle::GetSharedInstance();
  if (item_url == GURL("chrome://wallet/")) {
    if (focused)
      return *bundle.GetImageSkiaNamed(IDR_SIDEBAR_CRYPTO_WALLET_FOCUSED);
    return gfx::CreateVectorIcon(kSidebarCryptoWalletIcon, base_button_color);
  }

  if (item_url == GURL("https://together.brave.com/")) {
    if (focused)
      return *bundle.GetImageSkiaNamed(IDR_SIDEBAR_BRAVE_TOGETHER_FOCUSED);
    return gfx::CreateVectorIcon(kSidebarBraveTogetherIcon, base_button_color);
  }

  if (item_url == GURL("chrome://bookmarks/")) {
    if (focused)
      return *bundle.GetImageSkiaNamed(IDR_SIDEBAR_BOOKMARKS_FOCUSED);
    return gfx::CreateVectorIcon(kSidebarBookmarksIcon, base_button_color);
  }

  if (item_url == GURL("chrome://history/")) {
    if (focused)
      return *bundle.GetImageSkiaNamed(IDR_SIDEBAR_HISTORY_FOCUSED);
    return gfx::CreateVectorIcon(kSidebarHistoryIcon, base_button_color);
  }

  NOTREACHED();
  return gfx::ImageSkia();
}
