/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/sidebar/sidebar_control_view.h"

#include "brave/app/vector_icons/vector_icons.h"
#include "brave/browser/themes/theme_properties.h"
#include "brave/browser/ui/brave_browser.h"
#include "brave/browser/ui/sidebar/sidebar_controller.h"
#include "brave/browser/ui/sidebar/sidebar_utils.h"
#include "brave/browser/ui/views/sidebar/sidebar_add_item_bubble_delegate_view.h"
#include "brave/browser/ui/views/sidebar/sidebar_button_view.h"
#include "brave/browser/ui/views/sidebar/sidebar_items_container_view.h"
#include "brave/grit/brave_generated_resources.h"
#include "brave/grit/brave_theme_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/theme_provider.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/layout/box_layout.h"

namespace {

// To use bold font for title at index 0.
class ControlViewMenuModel : public ui::SimpleMenuModel {
 public:
  using SimpleMenuModel::SimpleMenuModel;
  ~ControlViewMenuModel() override = default;
  ControlViewMenuModel(const ControlViewMenuModel&) = delete;
  ControlViewMenuModel& operator=(const ControlViewMenuModel&) = delete;

  // ui::SimpleMenuModel overrides:
  const gfx::FontList* GetLabelFontListAt(int index) const override {
    if (index == 0) {
      return &ui::ResourceBundle::GetSharedInstance().GetFontList(
          ui::ResourceBundle::BoldFont);
    }
    return SimpleMenuModel::GetLabelFontListAt(index);
  }
};

}  // namespace

SidebarControlView::SidebarControlView(BraveBrowser* browser)
    : browser_(browser) {
  set_context_menu_controller(this);

  box_layout_ = SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical));

  AddChildViews();
  UpdateItemAddButtonState();
  UpdateSettingsButtonState();

  sidebar_model_observed_.Add(browser_->sidebar_controller()->model());
}

void SidebarControlView::OnThemeChanged() {
  View::OnThemeChanged();

  UpdateBackgroundAndBorder();
  UpdateItemAddButtonState();
  UpdateSettingsButtonState();
}

void SidebarControlView::UpdateBackgroundAndBorder() {
  if (const ui::ThemeProvider* theme_provider = GetThemeProvider()) {
    constexpr int kBorderThickness = 1;
    SetBackground(views::CreateSolidBackground(theme_provider->GetColor(
        BraveThemeProperties::COLOR_SIDEBAR_BACKGROUND)));
    SetBorder(views::CreateSolidSidedBorder(
        0, 0, 0, kBorderThickness,
        theme_provider->GetColor(BraveThemeProperties::COLOR_SIDEBAR_BORDER)));
  }
}

SidebarControlView::~SidebarControlView() = default;

void SidebarControlView::ShowContextMenuForViewImpl(
    views::View* source,
    const gfx::Point& point,
    ui::MenuSourceType source_type) {
  if (context_menu_runner_ && context_menu_runner_->IsRunning())
    return;

  context_menu_model_ = std::make_unique<ControlViewMenuModel>(this);
  context_menu_model_->AddTitle(
      l10n_util::GetStringUTF16(IDS_SIDEBAR_SHOW_OPTION_TITLE));
  context_menu_model_->AddCheckItem(
      kShowSidebarAlways,
      l10n_util::GetStringUTF16(IDS_SIDEBAR_SHOW_OPTION_ALWAYS));
  context_menu_model_->AddCheckItem(
      kShowSidebarOnMouseOver,
      l10n_util::GetStringUTF16(IDS_SIDEBAR_SHOW_OPTION_MOUSEOVER));
  context_menu_model_->AddCheckItem(
      kShowSidebarOnClick,
      l10n_util::GetStringUTF16(IDS_SIDEBAR_SHOW_OPTION_ONCLICK));
  context_menu_runner_ = std::make_unique<views::MenuRunner>(
      context_menu_model_.get(), views::MenuRunner::CONTEXT_MENU);
  context_menu_runner_->RunMenuAt(
      source->GetWidget(), nullptr, gfx::Rect(point, gfx::Size()),
      views::MenuAnchorPosition::kTopLeft, source_type);
}

void SidebarControlView::ExecuteCommand(int command_id, int event_flags) {
  NOTIMPLEMENTED();
}

bool SidebarControlView::IsCommandIdChecked(int command_id) const {
  return command_id == kShowSidebarAlways;
}

void SidebarControlView::OnItemAdded(const sidebar::SidebarItem& item,
                                     int index) {
  UpdateItemAddButtonState();
}

void SidebarControlView::OnItemRemoved(int index) {
  UpdateItemAddButtonState();
}

void SidebarControlView::AddChildViews() {
  sidebar_items_view_ =
      AddChildView(std::make_unique<SidebarItemsContainerView>(browser_));

  sidebar_item_add_view_ =
      AddChildView(std::make_unique<SidebarButtonView>(nullptr));
  sidebar_item_add_view_->set_context_menu_controller(this);
  sidebar_item_add_view_->SetCallback(
      base::BindRepeating(&SidebarControlView::OnButtonPressed,
                          base::Unretained(this), sidebar_item_add_view_));

  // This spacer view occupies the all empty space between add button and
  // settings button.
  auto* spacer = AddChildView(std::make_unique<views::View>());
  box_layout_->SetFlexForView(spacer, 1);

  sidebar_settings_view_ =
      AddChildView(std::make_unique<SidebarButtonView>(nullptr));
  sidebar_settings_view_->SetCallback(
      base::BindRepeating(&SidebarControlView::OnButtonPressed,
                          base::Unretained(this), sidebar_settings_view_));
}

void SidebarControlView::OnButtonPressed(views::View* view) {
  if (view == sidebar_item_add_view_) {
    auto* bubble = views::BubbleDialogDelegateView::CreateBubble(
        new SidebarAddItemBubbleDelegateView(browser_, view));
    bubble->Show();
    return;
  }

  if (view == sidebar_settings_view_) {
    // TODO(simonhong): Handle settings button here.
    NOTIMPLEMENTED();
  }
}

void SidebarControlView::Update() {
  UpdateItemAddButtonState();
}

void SidebarControlView::UpdateItemAddButtonState() {
  DCHECK(sidebar_item_add_view_);
  // Determine add button enabled state.
  bool should_enable = true;
  if (browser_->sidebar_controller()->model()->IsSidebarHasAllBuiltiInItems() &&
      !sidebar::CanAddCurrentActiveTabToSidebar(browser_)) {
    should_enable = false;
  }

  SkColor base_button_color = SK_ColorWHITE;
  if (const ui::ThemeProvider* theme_provider = GetThemeProvider()) {
    base_button_color = theme_provider->GetColor(
        BraveThemeProperties::COLOR_SIDEBAR_BUTTON_BASE);
  }

  // Update add button image based on enabled state.
  sidebar_item_add_view_->SetImage(views::Button::STATE_NORMAL, nullptr);
  sidebar_item_add_view_->SetImage(views::Button::STATE_DISABLED, nullptr);
  sidebar_item_add_view_->SetImage(views::Button::STATE_HOVERED, nullptr);
  sidebar_item_add_view_->SetImage(views::Button::STATE_PRESSED, nullptr);
  auto& bundle = ui::ResourceBundle::GetSharedInstance();
  if (should_enable) {
    sidebar_item_add_view_->SetImage(
        views::Button::STATE_NORMAL,
        gfx::CreateVectorIcon(kSidebarAddItemIcon, base_button_color));
    sidebar_item_add_view_->SetImage(
        views::Button::STATE_HOVERED,
        bundle.GetImageSkiaNamed(IDR_SIDEBAR_ITEM_ADD_FOCUSED));
    sidebar_item_add_view_->SetImage(
        views::Button::STATE_PRESSED,
        bundle.GetImageSkiaNamed(IDR_SIDEBAR_ITEM_ADD_FOCUSED));
  } else {
    sidebar_item_add_view_->SetImage(
        views::Button::STATE_NORMAL,
        gfx::CreateVectorIcon(kSidebarAddItemIcon, base_button_color));
  }

  sidebar_item_add_view_->SetEnabled(should_enable);
}

void SidebarControlView::UpdateSettingsButtonState() {
  DCHECK(sidebar_settings_view_);
  if (const ui::ThemeProvider* theme_provider = GetThemeProvider()) {
    sidebar_settings_view_->SetImage(
        views::Button::STATE_NORMAL,
        gfx::CreateVectorIcon(
            kSidebarSettingsIcon,
            theme_provider->GetColor(
                BraveThemeProperties::COLOR_SIDEBAR_BUTTON_BASE)));
  }
}
