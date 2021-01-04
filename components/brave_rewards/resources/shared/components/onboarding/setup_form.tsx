/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { RewardsTourProps } from './rewards_tour_props'
import * as style from './setup_form.style'

/*

== TODO ==

- BAT/BAP string
- Locale strings
- Exchange currency

*/

const adsPerHourOptions = [1, 2, 3, 4, 5]

export function SetupForm (props: RewardsTourProps) {
  return (
    <style.root>
      <style.section>
        <style.label>
          How often do you want to see ads?
        </style.label>
        <style.sublabel>
          (ads per hour)
        </style.sublabel>
        <style.optionBar>
          {
            adsPerHourOptions.map((value) => {
              const className = 'large-text ' +
                (value === props.adsPerHour ? 'selected' : '')
              const onClick = () => {
                if (value !== props.adsPerHour) {
                  props.onAdsPerHourChanged(value)
                }
              }
              return (
                <button key={value} onClick={onClick} className={className}>
                  {value}
                </button>
              )
            })
          }
        </style.optionBar>
      </style.section>
      <style.section>
        <style.label>
          How much monthly support do you want to give to your favorite
          creators?
        </style.label>
        <style.sublabel>
          (from your ads earnings)
        </style.sublabel>
        <style.optionBar>
          {
            props.acAmountOptions.slice(0, 4).map((amount) => {
              const exchangeAmount = amount * props.exchangeRate
              const className = amount === props.acAmount ? 'selected' : ''
              const onClick = () => {
                if (amount !== props.acAmount) {
                  props.onAcAmountChanged(amount)
                }
              }
              return (
                <button key={amount} onClick={onClick} className={className}>
                  <style.acAmount>
                    {amount.toFixed(0)}
                  </style.acAmount>&nbsp;
                  <style.acCurrency>
                    BAT
                  </style.acCurrency>
                  <style.acExchangeAmount>
                    {exchangeAmount.toFixed(2)} USD
                  </style.acExchangeAmount>
                </button>
              )
            })
          }
        </style.optionBar>
      </style.section>
    </style.root>
  )
}
