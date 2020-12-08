/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_LEDGER_TESTING_TEST_LEDGER_CLIENT_H_
#define BAT_LEDGER_TESTING_TEST_LEDGER_CLIENT_H_

#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "base/sequenced_task_runner.h"
#include "base/values.h"
#include "bat/ledger/internal/ledger_database_impl.h"
#include "bat/ledger/ledger_client.h"

namespace ledger {
namespace testing {

struct TestNetworkResult {
  TestNetworkResult(const std::string& url,
                    type::UrlMethod method,
                    type::UrlResponsePtr response)
      : url(url), method(method), response(std::move(response)) {}

  std::string url;
  type::UrlMethod method;
  type::UrlResponsePtr response;
};

class TestLedgerClient : public LedgerClient {
 public:
  TestLedgerClient();

  TestLedgerClient(const TestLedgerClient&) = delete;
  TestLedgerClient& operator=(const TestLedgerClient&) = delete;

  ~TestLedgerClient() override;

  void OnReconcileComplete(const type::Result result,
                           type::ContributionInfoPtr contribution) override;

  void LoadLedgerState(client::OnLoadCallback callback) override;

  void LoadPublisherState(client::OnLoadCallback callback) override;

  void OnPanelPublisherInfo(type::Result result,
                            type::PublisherInfoPtr publisher_info,
                            uint64_t windowId) override;

  void FetchFavIcon(const std::string& url,
                    const std::string& favicon_key,
                    client::FetchIconCallback callback) override;

  std::string URIEncode(const std::string& value) override;

  void LoadURL(type::UrlRequestPtr request,
               client::LoadURLCallback callback) override;

  void Log(const char* file,
           const int line,
           const int verbose_level,
           const std::string& message) override;

  void PublisherListNormalized(type::PublisherInfoList list) override;

  void SetBooleanState(const std::string& name, bool value) override;

  bool GetBooleanState(const std::string& name) const override;

  void SetIntegerState(const std::string& name, int value) override;

  int GetIntegerState(const std::string& name) const override;

  void SetDoubleState(const std::string& name, double value) override;

  double GetDoubleState(const std::string& name) const override;

  void SetStringState(const std::string& name,
                      const std::string& value) override;

  std::string GetStringState(const std::string& name) const override;

  void SetInt64State(const std::string& name, int64_t value) override;

  int64_t GetInt64State(const std::string& name) const override;

  void SetUint64State(const std::string& name, uint64_t value) override;

  uint64_t GetUint64State(const std::string& name) const override;

  void ClearState(const std::string& name) override;

  bool GetBooleanOption(const std::string& name) const override;

  int GetIntegerOption(const std::string& name) const override;

  double GetDoubleOption(const std::string& name) const override;

  std::string GetStringOption(const std::string& name) const override;

  int64_t GetInt64Option(const std::string& name) const override;

  uint64_t GetUint64Option(const std::string& name) const override;

  void OnContributeUnverifiedPublishers(
      type::Result result,
      const std::string& publisher_key,
      const std::string& publisher_name) override;

  std::string GetLegacyWallet() override;

  void ShowNotification(const std::string& type,
                        const std::vector<std::string>& args,
                        client::ResultCallback callback) override;

  type::ClientInfoPtr GetClientInfo() override;

  void UnblindedTokensReady() override;

  void ReconcileStampReset() override;

  void RunDBTransaction(type::DBTransactionPtr transaction,
                        client::RunDBTransactionCallback callback) override;

  void GetCreateScript(client::GetCreateScriptCallback callback) override;

  void PendingContributionSaved(const type::Result result) override;

  void ClearAllNotifications() override;

  void WalletDisconnected(const std::string& wallet_type) override;

  void DeleteLog(client::ResultCallback callback) override;

  bool SetEncryptedStringState(const std::string& name,
                               const std::string& value) override;

  std::string GetEncryptedStringState(const std::string& name) override;

  // Test environment setup methods:

  void SetOptionForTesting(const std::string& name, base::Value&& value);

  void SetNetworkResultsForTesting(std::list<TestNetworkResult>&& list);

  LedgerDatabaseImpl* database() { return ledger_database_.get(); }

 private:
  void LoadURLAfterDelay(type::UrlRequestPtr request,
                         client::LoadURLCallback callback);

  void RunDBTransactionCompleted(client::RunDBTransactionCallback callback,
                                 type::DBCommandResponsePtr response);

  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  std::unique_ptr<LedgerDatabaseImpl> ledger_database_;
  base::Value state_store_;
  base::Value encrypted_state_store_;
  base::Value option_store_;
  std::list<TestNetworkResult> network_results_;
  base::WeakPtrFactory<TestLedgerClient> weak_factory_{this};
};

}  // namespace testing
}  // namespace ledger

#endif  // BAT_LEDGER_TESTING_TEST_LEDGER_CLIENT_H_
