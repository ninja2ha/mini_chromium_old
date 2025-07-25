// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_VERSION_PE_VERSION_INFO_MAC_H_
#define MINI_CHROMIUM_SRC_CRBASE_VERSION_PE_VERSION_INFO_MAC_H_

#include <CoreFoundation/CoreFoundation.h>
#include <string>

#include "crbase//version/pe_version_info.h"
#include "crbase/mac/scoped_nsobject.h"

@class NSBundle;

namespace cr {

class PeFileVersionInfoMac : public PeFileVersionInfo {
 public:
  explicit PeFileVersionInfoMac(NSBundle *bundle);
  PeFileVersionInfoMac(const PeFileVersionInfoMac&) = delete;
  PeFileVersionInfoMac& operator=(const PeFileVersionInfoMac&) = delete;
  ~PeFileVersionInfoMac() override;

  // Accessors to the different version properties.
  // Returns an empty string if the property is not found.
  std::u16string company_name() override;
  std::u16string company_short_name() override;
  std::u16string product_name() override;
  std::u16string product_short_name() override;
  std::u16string internal_name() override;
  std::u16string product_version() override;
  std::u16string special_build() override;
  std::u16string original_filename() override;
  std::u16string file_description() override;
  std::u16string file_version() override;

 private:
  // Returns a std::u16string value for a property name.
  // Returns the empty string if the property does not exist.
  std::u16string GetString16Value(CFStringRef name);

  mac::scoped_nsobject<NSBundle> bundle_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_VERSION_PE_VERSION_INFO_MAC_H_
