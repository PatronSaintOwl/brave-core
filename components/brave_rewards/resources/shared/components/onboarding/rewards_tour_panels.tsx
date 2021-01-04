/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { Locale, formatMessageParts } from '../../lib/locale_context'
import { RewardsTourProps } from './rewards_tour_props'
import { SetupForm } from './setup_form'

import * as style from './rewards_tour_panels.style'

type TourPanelFunction = (locale: Locale, props: RewardsTourProps) => ({
  id: string
  heading: React.ReactNode
  text: React.ReactNode
  content?: React.ReactNode
})

function panelWelcome (locale: Locale) {
  const { getString } = locale
  return {
    id: 'welcome',
    heading: getString('onboardingPanelWelcomeHeader'),
    text: getString('onboardingPanelWelcomeText')
  }
}

function panelAds (locale: Locale) {
  const { getString } = locale
  return {
    id: 'ads',
    heading: getString('onboardingPanelAdsHeader'),
    text: getString('onboardingPanelAdsText')
  }
}

function panelSchedule (locale: Locale) {
  const { getString } = locale
  return {
    id: 'schedule',
    heading: getString('onboardingPanelScheduleHeader'),
    text: getString('onboardingPanelScheduleText')
  }
}

function panelAC (locale: Locale) {
  const { getString } = locale
  return {
    id: 'ac',
    heading: getString('onboardingPanelAcHeader'),
    text: getString('onboardingPanelAcText')
  }
}

function panelTipping (locale: Locale) {
  const { getString } = locale
  return {
    id: 'tipping',
    heading: getString('onboardingPanelTippingHeader'),
    text: getString('onboardingPanelTippingText')
  }
}

function panelRedeem (locale: Locale) {
  const { getString } = locale
  return {
    id: 'redeem',
    heading: getString('onboardingPanelRedeemHeader'),
    text: getString('onboardingPanelRedeemText')
  }
}

function panelSetup (locale: Locale, props: RewardsTourProps) {
  return {
    id: 'setup',
    heading: 'Lastly, let’s get you set up on the basic settings',
    text: <style.formText>You can change this later.</style.formText>,
    content: <SetupForm {...props} />
  }
}

function panelComplete (locale: Locale) {
  const { getString } = locale
  const heading = formatMessageParts(getString('onboardingPanelCompleteHeader'),
    (top, bottom) => <>{top}<br />{bottom}</>)
  return {
    id: 'complete',
    heading,
    text: getString('onboardingPanelCompleteText')
  }
}

export function getTourPanels (props: RewardsTourProps): TourPanelFunction[] {
  return [
    panelWelcome,
    panelAds,
    panelSchedule,
    panelAC,
    panelTipping,
    panelRedeem,
    ...(props.firstTimeSetup ? [panelSetup] : []),
    panelComplete
  ]
}
