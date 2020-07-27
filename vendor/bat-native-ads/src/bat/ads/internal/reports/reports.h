/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_REPORTS_H_
#define BAT_ADS_INTERNAL_REPORTS_H_

#include <string>

#include "bat/ads/ad_notification_info.h"
#include "bat/ads/confirmation_type.h"
#include "bat/ads/internal/reports/event_type_load_info.h"
#include "bat/ads/mojom.h"

namespace ads {

class AdsImpl;

class Reports {
 public:
  Reports(
      AdsImpl* ads);

  ~Reports();

  std::string GenerateAdNotificationEventReport(
      const AdNotificationInfo& info,
      const AdNotificationEventType event_type) const;

  std::string GenerateLoadEventReport(
      const LoadInfo& info) const;

  std::string GenerateSettingsEventReport() const;

 private:
  AdsImpl* ads_;  // NOT OWNED
};

}  // namespace ads

#endif  // BAT_ADS_INTERNAL_REPORTS_H_