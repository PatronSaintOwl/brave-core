/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.ntp_background_images.util;

import static org.chromium.ui.base.ViewUtils.dpToPx;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.ForegroundColorSpan;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.cardview.widget.CardView;

import org.chromium.base.ContextUtils;
import org.chromium.base.Log;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.BraveAdsNativeHelper;
import org.chromium.chrome.browser.BraveRewardsHelper;
import org.chromium.chrome.browser.BraveRewardsNativeWorker;
import org.chromium.chrome.browser.BraveRewardsPanelPopup;
import org.chromium.chrome.browser.app.BraveActivity;
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.browser.ntp_background_images.NTPBackgroundImagesBridge;
import org.chromium.chrome.browser.ntp_background_images.RewardsBottomSheetDialogFragment;
import org.chromium.chrome.browser.ntp_background_images.model.BackgroundImage;
import org.chromium.chrome.browser.ntp_background_images.model.NTPImage;
import org.chromium.chrome.browser.ntp_background_images.model.SponsoredTab;
import org.chromium.chrome.browser.ntp_background_images.model.Wallpaper;
import org.chromium.chrome.browser.ntp_background_images.util.SponsoredImageUtil;
import org.chromium.chrome.browser.preferences.BravePref;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.settings.BackgroundImagesPreferences;
import org.chromium.chrome.browser.tab.TabLaunchType;
import org.chromium.chrome.browser.util.ConfigurationUtils;
import org.chromium.chrome.browser.util.ImageUtils;
import org.chromium.chrome.browser.util.PackageUtils;
import org.chromium.components.user_prefs.UserPrefs;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.ui.base.DeviceFormFactor;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.SoftReference;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

public class NTPUtil {
    private static final int BOTTOM_TOOLBAR_HEIGHT = 56;
    private static final String REMOVED_SITES = "removed_sites";

    public static HashMap<String, SoftReference<Bitmap>> imageCache =
        new HashMap<String, SoftReference<Bitmap>>();

    public static void turnOnAds() {
        BraveAdsNativeHelper.nativeSetAdsEnabled(Profile.getLastUsedRegularProfile());
        BraveRewardsNativeWorker.getInstance().SetAutoContributeEnabled(true);
    }

    public static void updateOrientedUI(Context context, ViewGroup view, Point size) {
        LinearLayout parentLayout = (LinearLayout)view.findViewById(R.id.parent_layout);
        ViewGroup mainLayout = view.findViewById(R.id.ntp_main_layout);
        ViewGroup imageCreditLayout = view.findViewById(R.id.image_credit_layout);

        ImageView sponsoredLogo = (ImageView)view.findViewById(R.id.sponsored_logo);
        FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(dpToPx(context, 170), dpToPx(context, 170));

        parentLayout.removeView(mainLayout);
        parentLayout.removeView(imageCreditLayout);

        parentLayout.addView(mainLayout);
        parentLayout.addView(imageCreditLayout);

        parentLayout.setOrientation(LinearLayout.VERTICAL);

        boolean isTablet = DeviceFormFactor.isNonMultiDisplayContextOnTablet(context);
        DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
        float dpWidth = displayMetrics.widthPixels / displayMetrics.density;
        CardView widgetLayout = (CardView) view.findViewById(R.id.ntp_widget_cardview_layout);
        LinearLayout.LayoutParams widgetLayoutParams = new LinearLayout.LayoutParams(
                (isTablet ? (int) (dpWidth * 0.75) : dpToPx(context, 385)), dpToPx(context, 140));
        widgetLayout.setLayoutParams(widgetLayoutParams);

        LinearLayout.LayoutParams mainLayoutLayoutParams =
                new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, 0);
        mainLayoutLayoutParams.weight = 1f;
        mainLayout.setLayoutParams(mainLayoutLayoutParams);

        LinearLayout.LayoutParams imageCreditLayoutParams = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        imageCreditLayout.setLayoutParams(imageCreditLayoutParams);

        layoutParams.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        sponsoredLogo.setLayoutParams(layoutParams);
    }

    public static int checkForNonDisruptiveBanner(NTPImage ntpImage, SponsoredTab sponsoredTab) {
        Context context = ContextUtils.getApplicationContext();
        if(sponsoredTab.shouldShowBanner()) {
            if(PackageUtils.isFirstInstall(context)
                && ntpImage instanceof Wallpaper
                && !BraveAdsNativeHelper.nativeIsBraveAdsEnabled(Profile.getLastUsedRegularProfile())) {
                return SponsoredImageUtil.BR_ON_ADS_OFF ;
            } else if (ntpImage instanceof Wallpaper
                    && BraveAdsNativeHelper.nativeIsBraveAdsEnabled(
                            Profile.getLastUsedRegularProfile())) {
                return SponsoredImageUtil.BR_ON_ADS_ON;
            }
        }
        return SponsoredImageUtil.BR_INVALID_OPTION;
    }

    public static void showNonDisruptiveBanner(ChromeActivity chromeActivity, View view, int ntpType, SponsoredTab sponsoredTab, NewTabPageListener newTabPageListener) {
        final ViewGroup nonDisruptiveBannerLayout = (ViewGroup) view.findViewById(R.id.non_disruptive_banner);
        nonDisruptiveBannerLayout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (BraveAdsNativeHelper.nativeIsBraveAdsEnabled(
                            Profile.getLastUsedRegularProfile())) {
                    clickOnBottomBanner(chromeActivity, ntpType, nonDisruptiveBannerLayout,
                            sponsoredTab, newTabPageListener);
                } else {
                    if (BraveActivity.getBraveActivity() != null) {
                        nonDisruptiveBannerLayout.setVisibility(View.GONE);
                        BraveActivity.getBraveActivity().openRewardsPanel();
                    }
                }
                sponsoredTab.updateBannerPref();
            }
        });
        nonDisruptiveBannerLayout.setVisibility(View.GONE);

        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                BackgroundImagesPreferences.setOnPreferenceValue(BackgroundImagesPreferences.PREF_SHOW_NON_DISRUPTIVE_BANNER, false);

                boolean isTablet = DeviceFormFactor.isNonMultiDisplayContextOnTablet(chromeActivity);
                if (isTablet || (!isTablet && ConfigurationUtils.isLandscape(chromeActivity))) {
                    FrameLayout.LayoutParams nonDisruptiveBannerLayoutParams = new FrameLayout.LayoutParams(dpToPx(chromeActivity, 400), FrameLayout.LayoutParams.WRAP_CONTENT);
                    nonDisruptiveBannerLayoutParams.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
                    nonDisruptiveBannerLayout.setLayoutParams(nonDisruptiveBannerLayoutParams);
                } else {
                    FrameLayout.LayoutParams nonDisruptiveBannerLayoutParams = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.WRAP_CONTENT);
                    nonDisruptiveBannerLayoutParams.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
                    nonDisruptiveBannerLayout.setLayoutParams(nonDisruptiveBannerLayoutParams);
                }
                nonDisruptiveBannerLayout.setVisibility(View.VISIBLE);

                TextView bannerHeader = nonDisruptiveBannerLayout.findViewById(R.id.ntp_banner_header);
                TextView bannerText = nonDisruptiveBannerLayout.findViewById(R.id.ntp_banner_text);
                Button turnOnAdsButton = nonDisruptiveBannerLayout.findViewById(R.id.btn_turn_on_ads);
                ImageView bannerClose = nonDisruptiveBannerLayout.findViewById(R.id.ntp_banner_close);
                bannerClose.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        nonDisruptiveBannerLayout.setVisibility(View.GONE);
                        sponsoredTab.updateBannerPref();
                    }
                });

                turnOnAdsButton.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        NTPUtil.turnOnAds();
                        nonDisruptiveBannerLayout.setVisibility(View.GONE);

                        sponsoredTab.updateBannerPref();
                    }
                });

                switch (ntpType) {
                case SponsoredImageUtil.BR_OFF:
                    bannerText.setText(chromeActivity.getResources().getString(R.string.get_paid_to_see_image));
                    break;
                case SponsoredImageUtil.BR_ON_ADS_OFF:
                    bannerText.setText(getBannerText(chromeActivity, ntpType, nonDisruptiveBannerLayout, sponsoredTab, newTabPageListener));
                    break;
                case SponsoredImageUtil.BR_ON_ADS_OFF_BG_IMAGE:
                    bannerText.setText(chromeActivity.getResources().getString(R.string.you_can_support_creators));
                    turnOnAdsButton.setVisibility(View.VISIBLE);
                    break;
                case SponsoredImageUtil.BR_ON_ADS_ON:
                    bannerText.setText(getBannerText(chromeActivity, ntpType, nonDisruptiveBannerLayout, sponsoredTab, newTabPageListener));
                    break;
                }
            }
        }, 1500);
    }

    private static SpannableString getBannerText(ChromeActivity chromeActivity, int ntpType,
            View bannerLayout, SponsoredTab sponsoredTab, NewTabPageListener newTabPageListener) {
        String bannerText = "";
        if (ntpType == SponsoredImageUtil.BR_ON_ADS_ON) {
            bannerText = String.format(chromeActivity.getResources().getString(R.string.you_are_earning_tokens),
                                       chromeActivity.getResources().getString(R.string.learn_more));
        } else if (ntpType == SponsoredImageUtil.BR_ON_ADS_OFF) {
            bannerText = String.format(chromeActivity.getResources().getString(R.string.earn_tokens_for_viewing),
                                       chromeActivity.getResources().getString(R.string.learn_more));
        }
        int learnMoreIndex = bannerText.indexOf(chromeActivity.getResources().getString(R.string.learn_more));
        Spanned learnMoreSpanned = BraveRewardsHelper.spannedFromHtmlString(bannerText);
        SpannableString learnMoreTextSS = new SpannableString(learnMoreSpanned.toString());

        ForegroundColorSpan brOffForegroundSpan = new ForegroundColorSpan(chromeActivity.getResources().getColor(R.color.brave_theme_color));
        learnMoreTextSS.setSpan(brOffForegroundSpan, learnMoreIndex, learnMoreIndex + chromeActivity.getResources().getString(R.string.learn_more).length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        return learnMoreTextSS;
    }

    private static void clickOnBottomBanner(ChromeActivity chromeActivity, int ntpType, View bannerLayout, SponsoredTab sponsoredTab, NewTabPageListener newTabPageListener) {
        bannerLayout.setVisibility(View.GONE);

        RewardsBottomSheetDialogFragment rewardsBottomSheetDialogFragment = RewardsBottomSheetDialogFragment.newInstance();
        Bundle bundle = new Bundle();
        bundle.putInt(SponsoredImageUtil.NTP_TYPE, ntpType);
        rewardsBottomSheetDialogFragment.setArguments(bundle);
        rewardsBottomSheetDialogFragment.setNewTabPageListener(newTabPageListener);
        rewardsBottomSheetDialogFragment.show(chromeActivity.getSupportFragmentManager(), "rewards_bottom_sheet_dialog_fragment");
        rewardsBottomSheetDialogFragment.setCancelable(false);
    }

    public static Bitmap getWallpaperBitmap(NTPImage ntpImage, int layoutWidth, int layoutHeight) {
        Context mContext = ContextUtils.getApplicationContext();

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
        options.inSampleSize = ImageUtils.calculateInSampleSize(options, layoutWidth, layoutHeight);
        options.inJustDecodeBounds = false;

        Bitmap imageBitmap = null;
        float centerPointX;
        float centerPointY;

        if (ntpImage instanceof Wallpaper) {
            Wallpaper mWallpaper = (Wallpaper) ntpImage;
            InputStream inputStream = null;
            try {
                Uri imageFileUri = Uri.parse("file://" + mWallpaper.getImagePath());
                inputStream = mContext.getContentResolver().openInputStream(imageFileUri);
                imageBitmap = BitmapFactory.decodeStream(inputStream, null, options);
                inputStream.close();
            } catch (IOException exc) {
                Log.e("NTP", exc.getMessage());
            } finally {
                try {
                    if (inputStream != null) {
                        inputStream.close();
                    }
                } catch (IOException exception) {
                    Log.e("NTP", exception.getMessage());
                    return null;
                }
            }
            centerPointX = mWallpaper.getFocalPointX() == 0 ? (imageBitmap.getWidth() / 2) : mWallpaper.getFocalPointX();
            centerPointY = mWallpaper.getFocalPointY() == 0 ? (imageBitmap.getHeight() / 2) : mWallpaper.getFocalPointY();
        } else {
            BackgroundImage mBackgroundImage = (BackgroundImage) ntpImage;
            imageBitmap = BitmapFactory.decodeResource(mContext.getResources(), mBackgroundImage.getImageDrawable(), options);

            centerPointX = mBackgroundImage.getCenterPointX();
            centerPointY = mBackgroundImage.getCenterPointY();
        }
        return getCalculatedBitmap(imageBitmap, centerPointX, centerPointY, layoutWidth, layoutHeight);
    }

    public static Bitmap getCalculatedBitmap(Bitmap imageBitmap, float centerPointX, float centerPointY, int layoutWidth, int layoutHeight) {
        float imageWidth = imageBitmap.getWidth();
        float imageHeight = imageBitmap.getHeight();

        float centerRatioX = centerPointX / imageWidth;

        float imageWHRatio = imageWidth / imageHeight;
        float imageHWRatio = imageHeight / imageWidth;

        int newImageWidth = (int) (layoutHeight * imageWHRatio);
        int newImageHeight = layoutHeight;

        if (newImageWidth < layoutWidth) {
            // Image is now too small so we need to adjust width and height based on
            // This covers landscape and strange tablet sizes.
            newImageWidth = layoutWidth;
            newImageHeight = (int) (newImageWidth * imageHWRatio);
        }

        int newCenterX = (int) (newImageWidth * centerRatioX);
        int startX = (int) (newCenterX - (layoutWidth / 2));
        if (newCenterX < layoutWidth / 2) {
            // Need to crop starting at 0 to newImageWidth - left aligned image
            startX = 0;
        } else if (newImageWidth - newCenterX < layoutWidth / 2) {
            // Need to crop right side of image - right aligned
            startX = newImageWidth - layoutWidth;
        }

        int startY = (newImageHeight - layoutHeight) / 2;
        if (centerPointY > 0) {
            float centerRatioY = centerPointY / imageHeight;
            newImageWidth = layoutWidth;
            newImageHeight = (int) (layoutWidth * imageHWRatio);

            if (newImageHeight < layoutHeight) {
                newImageHeight = layoutHeight;
                newImageWidth = (int) (newImageHeight * imageWHRatio);
            }

            int newCenterY = (int) (newImageHeight * centerRatioY);
            startY = (int) (newCenterY - (layoutHeight / 2));
            if (newCenterY < layoutHeight / 2) {
                // Need to crop starting at 0 to newImageWidth - left aligned image
                startY = 0;
            } else if (newImageHeight - newCenterY < layoutHeight / 2) {
                // Need to crop right side of image - right aligned
                startY = newImageHeight - layoutHeight;
            }
        }

        imageBitmap = Bitmap.createScaledBitmap(imageBitmap, newImageWidth, newImageHeight, true);

        Bitmap newBitmap = Bitmap.createBitmap(imageBitmap, (startX + layoutWidth) <= imageBitmap.getWidth() ? startX : 0, (startY + (int) layoutHeight) <= imageBitmap.getHeight() ? startY : 0, layoutWidth, (int) layoutHeight);
        Bitmap bitmapWithGradient = ImageUtils.addGradient(newBitmap);

        imageBitmap.recycle();
        newBitmap.recycle();

        return bitmapWithGradient;
    }

    public static Bitmap getTopSiteBitmap(String iconPath) {
        Context mContext = ContextUtils.getApplicationContext();
        InputStream inputStream = null;
        Bitmap topSiteIcon = null;
        try {
            Uri imageFileUri = Uri.parse("file://" + iconPath);
            inputStream = mContext.getContentResolver().openInputStream(imageFileUri);
            topSiteIcon = BitmapFactory.decodeStream(inputStream);
            inputStream.close();
        } catch (IOException exc) {
            Log.e("NTP", exc.getMessage());
            topSiteIcon = null;
        } finally {
            try {
                if (inputStream != null) {
                    inputStream.close();
                }
            } catch (IOException exception) {
                Log.e("NTP", exception.getMessage());
                topSiteIcon = null;
            }
        }
        return topSiteIcon;
    }

    private static Set<String> getRemovedTopSiteUrls() {
        SharedPreferences mSharedPreferences = ContextUtils.getAppSharedPreferences();
        return mSharedPreferences.getStringSet(REMOVED_SITES, new HashSet<String>());
    }

    public static boolean isInRemovedTopSite(String url) {
        Set<String> urlSet = getRemovedTopSiteUrls();
        if (urlSet.contains(url)) {
            return true;
        }
        return false;
    }

    public static void addToRemovedTopSite(String url) {
        Set<String> urlSet = getRemovedTopSiteUrls();
        if (!urlSet.contains(url)) {
            urlSet.add(url);
        }

        SharedPreferences mSharedPreferences = ContextUtils.getAppSharedPreferences();
        SharedPreferences.Editor sharedPreferencesEditor = mSharedPreferences.edit();
        sharedPreferencesEditor.putStringSet(REMOVED_SITES, urlSet);
        sharedPreferencesEditor.apply();
    }

    public static boolean shouldEnableNTPFeature() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return true;
        }
        return false;
    }

    public static NTPImage getNTPImage(NTPBackgroundImagesBridge mNTPBackgroundImagesBridge) {
        Wallpaper mWallpaper = mNTPBackgroundImagesBridge.getCurrentWallpaper();
        if (mWallpaper != null) {
            return mWallpaper;
        } else {
            return SponsoredImageUtil.getBackgroundImage();
        }
    }

    public static boolean isReferralEnabled() {
        Profile mProfile = Profile.getLastUsedRegularProfile();
        NTPBackgroundImagesBridge mNTPBackgroundImagesBridge = NTPBackgroundImagesBridge.getInstance(mProfile);
        boolean isReferralEnabled = UserPrefs.get(Profile.getLastUsedRegularProfile()).getInteger(BravePref.NEW_TAB_PAGE_SUPER_REFERRAL_THEMES_OPTION) == 1 ? true : false;
        return mNTPBackgroundImagesBridge.isSuperReferral() && isReferralEnabled;
    }
}
