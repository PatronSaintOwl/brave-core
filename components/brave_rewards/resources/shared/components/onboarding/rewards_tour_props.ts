/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

export interface RewardsTourProps {
  layout?: 'narrow' | 'wide'
  firstTimeSetup: boolean
  adsPerHour: number
  acAmount: number
  acAmountOptions: number[]
  exchangeRate: number
  onAdsPerHourChanged: (adsPerHour: number) => void
  onAcAmountChanged: (acAmount: number) => void
  onDone: () => void
}
