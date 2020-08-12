/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/endpoint/promotion/promotion_server.h"

namespace ledger {
namespace endpoint {

PromotionServer::PromotionServer(bat_ledger::LedgerImpl* ledger):
    ledger_(ledger),
    get_available_(new promotion::GetAvailable(ledger)) {
  DCHECK(ledger_);
}

PromotionServer::~PromotionServer() = default;

promotion::GetAvailable* PromotionServer::get_available() const {
  return get_available_.get();
}

}  // namespace endpoint
}  // namespace ledger
