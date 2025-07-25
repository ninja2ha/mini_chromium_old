// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_VERSION_PE_FILE_VERSION_INFO_WIN_H_
#define MINI_CHROMIUM_SRC_CRBASE_VERSION_PE_FILE_VERSION_INFO_WIN_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "crbase/base_export.h"
#include "crbase/version/version.h"
#include "crbase/version/pe_file_version_info.h"
#include "crbase/win/win_util.h"

struct tagVS_FIXEDFILEINFO;
typedef tagVS_FIXEDFILEINFO VS_FIXEDFILEINFO;

namespace cr {

class CRBASE_EXPORT PeFileVersionInfoWin : public PeFileVersionInfo {
 public:
  PeFileVersionInfoWin(const PeFileVersionInfoWin&) = delete;
  PeFileVersionInfoWin& operator=(const PeFileVersionInfoWin&) = delete;
  ~PeFileVersionInfoWin() override;

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

  // Lets you access other properties not covered above. |value| is only
  // modified if GetValue() returns true.
  bool GetValue(const char16_t* name, std::u16string* value) const;

  // Similar to GetValue but returns a std::u16string (empty string if the
  // property does not exist).
  std::u16string GetStringValue(const char16_t* name) const;

  // Get file version number in dotted version format.
  Version GetFileVersion() const;

  // Behaves like CreatePEVersionInfo, but returns a PeFileVersionInfoWin.
  static std::unique_ptr<PeFileVersionInfoWin> CreatePeFileVersionInfoWin(
      const FilePath& file_path);

 private:
  friend PeFileVersionInfo;

  // |data| is a VS_VERSION_INFO resource. |language| and |code_page| are
  // extracted from the \VarFileInfo\Translation value of |data|.
  PeFileVersionInfoWin(std::vector<uint8_t>&& data,
                     WORD language,
                     WORD code_page);
  PeFileVersionInfoWin(void* data, WORD language, WORD code_page);

  const std::vector<uint8_t> owned_data_;
  const void* const data_;
  const WORD language_;
  const WORD code_page_;

  // This is a reference for a portion of |data_|.
  const VS_FIXEDFILEINFO& fixed_file_info_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_VERSION_PE_FILE_VERSION_INFO_WIN_H_