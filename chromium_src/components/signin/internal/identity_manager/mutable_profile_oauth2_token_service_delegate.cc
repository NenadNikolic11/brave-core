/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/signin/internal/identity_manager/mutable_profile_oauth2_token_service_delegate.h"

#define BRAVE_BLOCK_LOAD_CREDENTIALS \
  if (!account_tracker_service_->GetAccounts().size()) \
    return; \

#include "../../../../../../components/signin/internal/identity_manager/mutable_profile_oauth2_token_service_delegate.h"

