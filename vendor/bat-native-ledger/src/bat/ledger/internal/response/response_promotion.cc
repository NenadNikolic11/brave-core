/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/response/response_promotion.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "bat/ledger/internal/logging.h"
#include "bat/ledger/internal/promotion/promotion_util.h"
#include "net/http/http_status_code.h"

namespace braveledger_response_util {

// Request Url:
// POST /v1/promotions/{promotion_id}
//
// Success:
// OK (200)
//
// Response Format:
// {
//   "claimId": "53714048-9675-419e-baa3-369d85a2facb"
// }

ledger::Result ParseClaimCreds(
    const ledger::UrlResponse& response,
    std::string* claim_id) {
  DCHECK(claim_id);

  // Bad Request (400)
  if (response.status_code == net::HTTP_BAD_REQUEST) {
    BLOG(0, "Invalid request");
    return ledger::Result::LEDGER_ERROR;
  }

  // Forbidden (403)
  if (response.status_code == net::HTTP_FORBIDDEN) {
    BLOG(0, "Signature validation failed");
    return ledger::Result::LEDGER_ERROR;
  }

  // Conflict (409)
  if (response.status_code == net::HTTP_CONFLICT) {
    BLOG(0, "Incorrect blinded credentials");
    return ledger::Result::LEDGER_ERROR;
  }

  // Gone (410)
  if (response.status_code == net::HTTP_GONE) {
    BLOG(0, "Promotion is gone");
    return ledger::Result::NOT_FOUND;
  }

  // Internal Server Error (500)
  if (response.status_code == net::HTTP_INTERNAL_SERVER_ERROR) {
    BLOG(0, "Internal server error");
    return ledger::Result::LEDGER_ERROR;
  }

  if (response.status_code != net::HTTP_OK) {
    return ledger::Result::LEDGER_ERROR;
  }

  base::Optional<base::Value> value = base::JSONReader::Read(response.body);
  if (!value || !value->is_dict()) {
    BLOG(0, "Invalid JSON");
    return ledger::Result::LEDGER_ERROR;
  }

  base::DictionaryValue* dictionary = nullptr;
  if (!value->GetAsDictionary(&dictionary)) {
    BLOG(0, "Invalid JSON");
    return ledger::Result::LEDGER_ERROR;
  }

  auto* id = dictionary->FindStringKey("claimId");
  if (!id || id->empty()) {
    BLOG(0, "Claim id is missing");
    return ledger::Result::LEDGER_ERROR;
  }

  *claim_id = *id;

  return ledger::Result::LEDGER_OK;
}

// Request Url:
// POST /v1/promotions/reportclobberedclaims
//
// Success:
// OK (200)
//
// Response Format:
// {Empty body}

ledger::Result CheckCorruptedPromotions(const ledger::UrlResponse& response) {
  // Bad Request (400)
  if (response.status_code == net::HTTP_BAD_REQUEST) {
    BLOG(0, "Invalid request");
    return ledger::Result::LEDGER_ERROR;
  }

  // Internal Server Error (500)
  if (response.status_code == net::HTTP_INTERNAL_SERVER_ERROR) {
    BLOG(0, "Internal server error");
    return ledger::Result::LEDGER_ERROR;
  }

  if (response.status_code != net::HTTP_OK) {
    return ledger::Result::LEDGER_ERROR;
  }

  return ledger::Result::LEDGER_OK;
}

// Request Url:
// POST /v1/suggestions
//
// Success:
// OK (200)
//
// Response Format:
// {Empty body}

ledger::Result CheckRedeemTokens(const ledger::UrlResponse& response) {
  // Bad Request (400)
  if (response.status_code == net::HTTP_BAD_REQUEST) {
    BLOG(0, "Invalid request");
    return ledger::Result::LEDGER_ERROR;
  }

  // Internal Server Error (500)
  if (response.status_code == net::HTTP_INTERNAL_SERVER_ERROR) {
    BLOG(0, "Internal server error");
    return ledger::Result::LEDGER_ERROR;
  }

  if (response.status_code != net::HTTP_OK) {
    return ledger::Result::LEDGER_ERROR;
  }

  return ledger::Result::LEDGER_OK;
}

}  // namespace braveledger_response_util
