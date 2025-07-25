// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/files/file_version_info_mac.h"

#import <Foundation/Foundation.h>

#include "crbase/memory/ptr_util.h"
#include "crbase/files/file_path.h"
#include "crbase/mac/bundle_locations.h"
#include "crbase/mac/foundation_util.h"
#include "crbase/strings/sys_string_conversions.h"
#include "crbase/compiler/build_config.h"

namespace cr {

PeFileVersionInfoMac::PeFileVersionInfoMac(NSBundle *bundle)
    : bundle_([bundle retain]) {
}

PeFileVersionInfoMac::~PeFileVersionInfoMac() {}

// static
std::unique_ptr<PEVersionInfo>
PEVersionInfo::CreatePEVersionInfoForCurrentModule() {
  return CreatePEVersionInfo(mac::FrameworkBundlePath());
}

// static
std::unique_ptr<PEVersionInfo> PEVersionInfo::CreatePeFileVersionInfo(
    const FilePath& file_path) {
  NSString* path = SysUTF8ToNSString(file_path.value());
  NSBundle* bundle = [NSBundle bundleWithPath:path];
  return cr::MakeUnique<PeFileVersionInfoMac>(bundle);
}

std::u16string PeFileVersionInfoMac::company_name() {
  return std::u16string();
}

std::u16string PeFileVersionInfoMac::company_short_name() {
  return std::u16string();
}

std::u16string PeFileVersionInfoMac::internal_name() {
  return std::u16string();
}

std::u16string PeFileVersionInfoMac::product_name() {
  return GetString16Value(kCFBundleNameKey);
}

std::u16string PeFileVersionInfoMac::product_short_name() {
  return GetString16Value(kCFBundleNameKey);
}

std::u16string PeFileVersionInfoMac::product_version() {
  // On OS X, CFBundleVersion is used by LaunchServices, and must follow
  // specific formatting rules, so the four-part Chrome version is in
  // CFBundleShortVersionString. On iOS, both have a policy-enfoced limit
  // of three version components, so the full version is stored in a custom
  // key (CrBundleVersion) falling back to CFBundleVersion if not present.
#if defined(MINI_CHROMIUM_OS_IOS)
  return GetString16Value(CFSTR("CFBundleVersion"));
#else
  return GetString16Value(CFSTR("CFBundleShortVersionString"));
#endif  // defined(MINI_CHROMIUM_OS_IOS)
}

std::u16string PeFileVersionInfoMac::file_description() {
  return std::u16string();
}

std::u16string PeFileVersionInfoMac::file_version() {
  return product_version();
}

std::u16string PeFileVersionInfoMac::original_filename() {
  return GetString16Value(kCFBundleNameKey);
}

std::u16string PeFileVersionInfoMac::special_build() {
  return std::u16string();
}

std::u16string PeFileVersionInfoMac::GetString16Value(CFStringRef name) {
  if (bundle_) {
    NSString *ns_name = mac::CFToNSCast(name);
    NSString* value = [bundle_ objectForInfoDictionaryKey:ns_name];
    if (value) {
      return SysNSStringToUTF16(value);
    }
  }
  return std::u16string();
}

}  // namespace cr
