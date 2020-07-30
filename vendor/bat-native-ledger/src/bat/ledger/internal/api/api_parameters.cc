/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>
#include <utility>

#include "base/time/time.h"
#include "bat/ledger/internal/api/api_parameters.h"
#include "bat/ledger/internal/common/time_util.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/request/request_api.h"
#include "bat/ledger/internal/response/response_api.h"
#include "bat/ledger/internal/state/state_util.h"

using std::placeholders::_1;

namespace braveledger_api {

APIParameters::APIParameters(bat_ledger::LedgerImpl* ledger) :
    ledger_(ledger) {
  DCHECK(ledger_);
}

APIParameters::~APIParameters() = default;

void APIParameters::Initialize() {
  if (braveledger_state::GetRewardsMainEnabled(ledger_)) {
    Fetch([](ledger::RewardsParametersPtr) {});
  }
}

void APIParameters::Fetch(ledger::GetRewardsParametersCallback callback) {
  bool first_request = callbacks_.empty();
  callbacks_.push_back(callback);
  if (!first_request) {
    BLOG(1, "API parameters fetch in progress");
    return;
  }

  refresh_timer_.Stop();

  auto url_callback = std::bind(&APIParameters::OnFetch,
      this,
      _1);

  const std::string url = braveledger_request_util::GetParametersURL();
  ledger_->LoadURL(url, {}, "", "", ledger::UrlMethod::GET, url_callback);
}

void APIParameters::OnFetch(const ledger::UrlResponse& response) {
  BLOG(6, ledger::UrlResponseToString(__func__, response));

  ledger::RewardsParameters parameters;
  const ledger::Result result =
      braveledger_response_util::ParseParameters(response, &parameters);
  if (result == ledger::Result::RETRY_SHORT) {
    RunCallbacks();
    SetRefreshTimer(base::TimeDelta::FromSeconds(90));
    return;
  }

  if (result != ledger::Result::LEDGER_OK) {
    BLOG(1, "Couldn't parse response");
    RunCallbacks();
    SetRefreshTimer(base::TimeDelta::FromMinutes(10));
    return;
  }

  braveledger_state::SetRewardsParameters(ledger_, parameters);
  RunCallbacks();
  SetRefreshTimer(
      base::TimeDelta::FromMinutes(10),
      base::TimeDelta::FromHours(3));
}

void APIParameters::RunCallbacks() {
  // Execute callbacks with the current parameters stored in state.
  // If the last fetch failed, callbacks will be run with the last
  // successfully fetched parameters or a default set of parameters.
  auto parameters = braveledger_state::GetRewardsParameters(ledger_);
  DCHECK(parameters);

  auto callbacks = std::move(callbacks_);
  for (auto& callback : callbacks) {
    callback(parameters->Clone());
  }
}

void APIParameters::SetRefreshTimer(
    base::TimeDelta delay,
    base::TimeDelta base_delay) {
  if (refresh_timer_.IsRunning()) {
    BLOG(1, "Params timer in progress");
    return;
  }

  base::TimeDelta start_in =
      base_delay + braveledger_time_util::GetRandomizedDelay(delay);

  BLOG(1, "Params timer set for " << start_in);

  refresh_timer_.Start(FROM_HERE, start_in,
      base::BindOnce(&APIParameters::Fetch,
          base::Unretained(this),
          [](ledger::RewardsParametersPtr) {}));
}

}  // namespace braveledger_api
