/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>
#include <utility>

#include "base/run_loop.h"
#include "base/test/task_environment.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/testing/test_ledger_client.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ledger {
namespace testing {

// Calls the provided function with a completion callback and starts a run
// loop. The run loop is exited when the completion callback is executed. The
// argument provided to the completion callback is returned to the caller.
template <typename R, typename F>
R WaitFor(F fn) {
  R result;
  base::RunLoop run_loop;
  fn([&run_loop, &result](auto value) {
    result = std::move(value);
    run_loop.Quit();
  });
  run_loop.Run();
  return result;
}

// Returns the file path of the directory containing test data
base::FilePath GetTestDataPath();

class LedgerTest : public ::testing::Test {
 protected:
  LedgerTest();

  // Initializes the ledger API
  void InitializeLedger();

  // Executes an SQL command against the ledger database
  void ExecuteSQL(const std::string& sql);

  Ledger* ledger() { return &ledger_; }
  TestLedgerClient* client() { return &client_; }

  base::test::TaskEnvironment task_environment_;
  TestLedgerClient client_;
  LedgerImpl ledger_;
};

}  // namespace testing
}  // namespace ledger
