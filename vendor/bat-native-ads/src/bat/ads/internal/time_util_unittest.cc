/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/time_util.h"

#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

TEST(BatAdsTimeUtilTest,
    GetLocalDayOfWeekForMonday) {
  // Arrange
  const base::Time time = TimeFromDateString("16 November 1970");

  // Act
  const std::string local_week_day = GetLocalWeekDay(time);

  // Assert
  const std::string expected_local_week_day = "1";  // Monday

  EXPECT_EQ(expected_local_week_day, local_week_day);
}

TEST(BatAdsTimeUtilTest,
    GetLocalDayOfWeekForTuesday) {
  // Arrange
  const base::Time time = TimeFromDateString("17 November 1970");

  // Act
  const std::string local_week_day = GetLocalWeekDay(time);

  // Assert
  const std::string expected_local_week_day = "2";  // Tuesday

  EXPECT_EQ(expected_local_week_day, local_week_day);
}

TEST(BatAdsTimeUtilTest,
    GetLocalDayOfWeekForWednesday) {
  // Arrange
  const base::Time time = TimeFromDateString("18 November 1970");

  // Act
  const std::string local_week_day = GetLocalWeekDay(time);

  // Assert
  const std::string expected_local_week_day = "3";  // Wednesday

  EXPECT_EQ(expected_local_week_day, local_week_day);
}

TEST(BatAdsTimeUtilTest,
    GetLocalDayOfWeekForThursday) {
  // Arrange
  const base::Time time = TimeFromDateString("19 November 1970");

  // Act
  const std::string local_week_day = GetLocalWeekDay(time);

  // Assert
  const std::string expected_local_week_day = "4";  // Thursday

  EXPECT_EQ(expected_local_week_day, local_week_day);
}

TEST(BatAdsTimeUtilTest,
    GetLocalDayOfWeekForFriday) {
  // Arrange
  const base::Time time = TimeFromDateString("20 November 1970");

  // Act
  const std::string local_week_day = GetLocalWeekDay(time);

  // Assert
  const std::string expected_local_week_day = "5";  // Friday

  EXPECT_EQ(expected_local_week_day, local_week_day);
}

TEST(BatAdsTimeUtilTest,
    GetLocalDayOfWeekForSaturday) {
  // Arrange
  const base::Time time = TimeFromDateString("21 November 1970");

  // Act
  const std::string local_week_day = GetLocalWeekDay(time);

  // Assert
  const std::string expected_local_week_day = "6";  // Saturday

  EXPECT_EQ(expected_local_week_day, local_week_day);
}

TEST(BatAdsTimeUtilTest,
    GetLocalDayOfWeekForSunday) {
  // Arrange
  const base::Time time = TimeFromDateString("22 November 1970");

  // Act
  const std::string local_week_day = GetLocalWeekDay(time);

  // Assert
  const std::string expected_local_week_day = "0";  // Sunday

  EXPECT_EQ(expected_local_week_day, local_week_day);
}

TEST(BatAdsTimeUtilTest,
    ConvertHoursAndMinutesToMinutesForUTC) {
  // Arrange
  const base::Time time =
      TimeFromDateString("18 November 1970 12:34:56.789 +00:00");

  // Act
  const int minutes = ConvertHoursAndMinutesToMinutes(time);

  // Assert
  const int expected_minutes = (12 * base::Time::kSecondsPerMinute) + 34;

  EXPECT_EQ(expected_minutes, minutes);
}

TEST(BatAdsTimeUtilTest,
    ConvertHoursAndMinutesToMinutesForNonUTC) {
  // Arrange
  const base::Time time =
      TimeFromDateString("18 November 1970 12:34:56.789 -06:00");

  // Act
  const int minutes = ConvertHoursAndMinutesToMinutes(time);

  // Assert
  const int expected_minutes = ((12 + 6) * base::Time::kSecondsPerMinute) + 34;

  EXPECT_EQ(expected_minutes, minutes);
}

}  // namespace ads
