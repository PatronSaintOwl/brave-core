/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/testing/test_ledger_client.h"

#include <utility>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "net/base/escape.h"
#include "net/http/http_status_code.h"

namespace {

ledger::type::DBCommandResponsePtr RunDBTransactionInTask(
    ledger::type::DBTransactionPtr transaction,
    ledger::LedgerDatabase* database) {
  DCHECK(database);
  auto response = ledger::type::DBCommandResponse::New();
  database->RunTransaction(std::move(transaction), response.get());
  return response;
}

}  // namespace

namespace ledger {
namespace testing {

TestLedgerClient::TestLedgerClient()
    : task_runner_(base::ThreadPool::CreateSequencedTaskRunner(
          {base::MayBlock(), base::TaskShutdownBehavior::BLOCK_SHUTDOWN})),
      ledger_database_(new LedgerDatabaseImpl(base::FilePath(""))),
      state_store_(base::Value::Type::DICTIONARY),
      encrypted_state_store_(base::Value::Type::DICTIONARY),
      option_store_(base::Value::Type::DICTIONARY) {
  CHECK(ledger_database_->OpenInMemoryForTesting());
}

TestLedgerClient::~TestLedgerClient() {
  task_runner_->DeleteSoon(FROM_HERE, ledger_database_.release());
}

void TestLedgerClient::OnReconcileComplete(
    const type::Result result,
    type::ContributionInfoPtr contribution) {}

void TestLedgerClient::LoadLedgerState(client::OnLoadCallback callback) {
  callback(type::Result::NO_LEDGER_STATE, "");
}

void TestLedgerClient::LoadPublisherState(client::OnLoadCallback callback) {
  callback(type::Result::NO_PUBLISHER_STATE, "");
}

void TestLedgerClient::OnPanelPublisherInfo(
    type::Result result,
    type::PublisherInfoPtr publisher_info,
    uint64_t windowId) {}

void TestLedgerClient::FetchFavIcon(const std::string& url,
                                    const std::string& favicon_key,
                                    client::FetchIconCallback callback) {
  callback(true, favicon_key);
}

std::string TestLedgerClient::URIEncode(const std::string& value) {
  return net::EscapeQueryParamValue(value, false);
}

void TestLedgerClient::LoadURL(type::UrlRequestPtr request,
                               client::LoadURLCallback callback) {
  DCHECK(request);
  task_runner_->PostTaskAndReply(
      FROM_HERE, base::DoNothing(),
      base::BindOnce(&TestLedgerClient::LoadURLAfterDelay,
                     weak_factory_.GetWeakPtr(), std::move(request), callback));
}

void TestLedgerClient::Log(const char* file,
                           const int line,
                           const int verbose_level,
                           const std::string& message) {
  int vlog_level = logging::GetVlogLevelHelper(file, strlen(file));
  if (verbose_level <= vlog_level) {
    logging::LogMessage(file, line, -verbose_level).stream() << message;
  }
}

void TestLedgerClient::PublisherListNormalized(type::PublisherInfoList list) {}

void TestLedgerClient::SetBooleanState(const std::string& name, bool value) {
  state_store_.SetBoolPath(name, value);
}

bool TestLedgerClient::GetBooleanState(const std::string& name) const {
  auto opt = state_store_.FindBoolPath(name);
  return opt ? *opt : false;
}

void TestLedgerClient::SetIntegerState(const std::string& name, int value) {
  state_store_.SetIntPath(name, value);
}

int TestLedgerClient::GetIntegerState(const std::string& name) const {
  auto opt = state_store_.FindIntPath(name);
  return opt ? *opt : 0;
}

void TestLedgerClient::SetDoubleState(const std::string& name, double value) {
  state_store_.SetDoublePath(name, value);
}

double TestLedgerClient::GetDoubleState(const std::string& name) const {
  auto opt = state_store_.FindDoublePath(name);
  return opt ? *opt : 0.0;
}

void TestLedgerClient::SetStringState(const std::string& name,
                                      const std::string& value) {
  state_store_.SetStringPath(name, value);
}

std::string TestLedgerClient::GetStringState(const std::string& name) const {
  auto opt = state_store_.FindStringPath(name);
  return opt ? *opt : "";
}

void TestLedgerClient::SetInt64State(const std::string& name, int64_t value) {
  state_store_.SetStringPath(name, base::NumberToString(value));
}

int64_t TestLedgerClient::GetInt64State(const std::string& name) const {
  if (auto opt = state_store_.FindStringPath(name)) {
    int64_t value;
    if (base::StringToInt64(*opt, &value)) {
      return value;
    }
  }
  return 0;
}

void TestLedgerClient::SetUint64State(const std::string& name, uint64_t value) {
  state_store_.SetStringPath(name, base::NumberToString(value));
}

uint64_t TestLedgerClient::GetUint64State(const std::string& name) const {
  if (auto opt = state_store_.FindStringPath(name)) {
    uint64_t value;
    if (base::StringToUint64(*opt, &value)) {
      return value;
    }
  }
  return 0;
}

void TestLedgerClient::ClearState(const std::string& name) {
  state_store_.RemovePath(name);
}

bool TestLedgerClient::GetBooleanOption(const std::string& name) const {
  auto opt = state_store_.FindBoolPath(name);
  return opt ? *opt : false;
}

int TestLedgerClient::GetIntegerOption(const std::string& name) const {
  auto opt = state_store_.FindIntPath(name);
  return opt ? *opt : 0;
}

double TestLedgerClient::GetDoubleOption(const std::string& name) const {
  auto opt = state_store_.FindDoublePath(name);
  return opt ? *opt : 0.0;
}

std::string TestLedgerClient::GetStringOption(const std::string& name) const {
  auto opt = state_store_.FindStringPath(name);
  return opt ? *opt : "";
}

int64_t TestLedgerClient::GetInt64Option(const std::string& name) const {
  if (auto opt = state_store_.FindStringPath(name)) {
    int64_t value;
    if (base::StringToInt64(*opt, &value)) {
      return value;
    }
  }
  return 0;
}

uint64_t TestLedgerClient::GetUint64Option(const std::string& name) const {
  if (auto opt = state_store_.FindStringPath(name)) {
    uint64_t value;
    if (base::StringToUint64(*opt, &value)) {
      return value;
    }
  }
  return 0;
}

void TestLedgerClient::OnContributeUnverifiedPublishers(
    type::Result result,
    const std::string& publisher_key,
    const std::string& publisher_name) {}

std::string TestLedgerClient::GetLegacyWallet() {
  // TODO(zenparsing): Should this return anything by default?
  return "";
}

void TestLedgerClient::ShowNotification(const std::string& type,
                                        const std::vector<std::string>& args,
                                        client::ResultCallback callback) {}

type::ClientInfoPtr TestLedgerClient::GetClientInfo() {
  auto info = type::ClientInfo::New();
  info->platform = type::Platform::DESKTOP;
  info->os = type::OperatingSystem::UNDEFINED;
  return info;
}

void TestLedgerClient::UnblindedTokensReady() {}

void TestLedgerClient::ReconcileStampReset() {}

void TestLedgerClient::RunDBTransaction(
    type::DBTransactionPtr transaction,
    client::RunDBTransactionCallback callback) {
  task_runner_->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(RunDBTransactionInTask, std::move(transaction),
                     ledger_database_.get()),
      base::BindOnce(&TestLedgerClient::RunDBTransactionCompleted,
                     weak_factory_.GetWeakPtr(), std::move(callback)));
}

void TestLedgerClient::GetCreateScript(
    client::GetCreateScriptCallback callback) {
  callback("", 0);
}

void TestLedgerClient::PendingContributionSaved(const type::Result result) {}

void TestLedgerClient::ClearAllNotifications() {}

void TestLedgerClient::WalletDisconnected(const std::string& wallet_type) {}

void TestLedgerClient::DeleteLog(client::ResultCallback callback) {
  callback(type::Result::LEDGER_OK);
}

bool TestLedgerClient::SetEncryptedStringState(const std::string& name,
                                               const std::string& value) {
  encrypted_state_store_.SetStringPath(name, value);
  return true;
}

std::string TestLedgerClient::GetEncryptedStringState(const std::string& name) {
  auto opt = encrypted_state_store_.FindStringPath(name);
  return opt ? *opt : "";
}

void TestLedgerClient::SetOptionForTesting(const std::string& name,
                                           base::Value&& value) {
  option_store_.SetPath(name, std::move(value));
}

void TestLedgerClient::SetNetworkResultsForTesting(
    std::list<TestNetworkResult>&& list) {
  network_results_ = std::move(list);
}

void TestLedgerClient::LoadURLAfterDelay(type::UrlRequestPtr request,
                                         client::LoadURLCallback callback) {
  auto iter = std::find_if(network_results_.begin(), network_results_.end(),
                           [&request](auto& result) {
                             return request->url == result.url &&
                                    request->method == result.method;
                           });

  if (iter != network_results_.end()) {
    callback(*iter->response);
    network_results_.erase(iter);
    return;
  }

  type::UrlResponse response;
  response.url = request->url;
  response.status_code = net::HTTP_BAD_REQUEST;
  callback(response);
}

void TestLedgerClient::RunDBTransactionCompleted(
    client::RunDBTransactionCallback callback,
    type::DBCommandResponsePtr response) {
  callback(std::move(response));
}

}  // namespace testing
}  // namespace ledger
