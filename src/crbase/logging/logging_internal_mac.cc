// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/logging/logging_internal_mac.h"

#include <AvailabilityMacros.h>
#include <CoreFoundation/CoreFoundation.h>

#if !defined(MAC_OS_X_VERSION_10_12) || \
    MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_12
#include <asl.h>
#define USE_APPLE_SYSTEM_LOG
#else
#include <os/log.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>
#include <sys/stat.h>

#include <vector>

#include "crbase/stl_util.h"

namespace {

bool LogToSystem() {
  struct stat stderr_stat;
  if (fstat(fileno(stderr), &stderr_stat) == -1) {
    return true;
  }
  if (!S_ISCHR(stderr_stat.st_mode)) {
    return false;
  }

  struct stat dev_null_stat;
  if (stat(_PATH_DEVNULL, &dev_null_stat) == -1) {
    return true;
  }

  return !S_ISCHR(dev_null_stat.st_mode) ||
         stderr_stat.st_rdev == dev_null_stat.st_rdev;
}

// Copy from chromium/base/strings/sys_string_conversions_mac.mm
std::string CFStringRefToUTF8(CFStringRef cfstring) {
  CFIndex length = CFStringGetLength(cfstring);
  if (length == 0)
    return std::string();

  CFRange whole_string = CFRangeMake(0, length);
  CFIndex out_size;
  CFIndex converted = CFStringGetBytes(cfstring,
                                       whole_string,
                                       kCFStringEncodingUTF8,
                                       0,      // lossByte
                                       false,  // isExternalRepresentation
                                       NULL,   // buffer
                                       0,      // maxBufLen
                                       &out_size);
  if (converted == 0 || out_size == 0)
    return std::string();

  // out_size is the number of UInt8-sized units needed in the destination.
  // A buffer allocated as UInt8 units might not be properly aligned to
  // contain elements of StringType::value_type.  Use a container for the
  // proper value_type, and convert out_size by figuring the number of
  // value_type elements per UInt8.  Leave room for a NUL terminator.
  size_t elements = out_size * sizeof(UInt8) + 1;

  std::vector<char> out_buffer(elements);
  converted = CFStringGetBytes(cfstring,
                               whole_string,
                               kCFStringEncodingUTF8,
                               0,      // lossByte
                               false,  // isExternalRepresentation
                               reinterpret_cast<UInt8*>(&out_buffer[0]),
                               out_size,
                               NULL);  // usedBufLen
  if (converted == 0)
    return std::string();

  out_buffer[elements - 1] = '\0';
  return std::string(&out_buffer[0], elements - 1);
}

#if defined(USE_APPLE_SYSTEM_LOG)
// The facility is set to the main bundle ID if available. Otherwise,
// "com.apple.console" is used.
class ASLClient {
 public:
  ASLClient(const ASLClient&) = delete;
  ASLClient& operator=(const ASLClient&) = delete;

  explicit ASLClient(const std::string& facility)
      : client_(asl_open(nullptr, facility.c_str(), ASL_OPT_NO_DELAY)) {}
  ~ASLClient() { asl_close(client_); }

  aslclient get() const { return client_; }

 private:
  aslclient client_;
};

class ASLMessage {
 public:
  ASLMessage(const ASLMessage&) = delete;
  ASLMessage& operator=(const ASLMessage&) = delete;

  ASLMessage() : message_(asl_new(ASL_TYPE_MSG)) {}
  ~ASLMessage() { asl_free(message_); }

  aslmsg get() const { return message_; }

 private:
  aslmsg message_;
};

const char* LogSeverityToASLLevelString(cr::logging::LogSeverity severity) {
  // ASL_LEVEL_* are ints, but ASL needs equivalent strings. This
  // non-obvious two-step macro trick achieves what's needed.
  // https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define ASL_LEVEL_STR(level) ASL_LEVEL_STR_X(level)
#define ASL_LEVEL_STR_X(level) #level
  switch (severity) {
    case cr::logging::kLogInfo:
      return ASL_LEVEL_STR(ASL_LEVEL_INFO);
    case cr::logging::kLogWarning:
      return ASL_LEVEL_STR(ASL_LEVEL_WARNING);
    case cr::logging::kLogError:
      return ASL_LEVEL_STR(ASL_LEVEL_ERR);
    case cr::logging::kLogFatal:
      return ASL_LEVEL_STR(ASL_LEVEL_CRIT);
    default:
      return severity < 0 ? ASL_LEVEL_STR(ASL_LEVEL_DEBUG)
                          : ASL_LEVEL_STR(ASL_LEVEL_NOTICE);
  }
#undef ASL_LEVEL_STR
#undef ASL_LEVEL_STR_X
}

#else

class OSLog {
 public:
  OSLog(const OSLog&) = delete;
  OSLog& operator=(const OSLog&) = delete;

  explicit OSLog(const char* subsystem)
      : os_log_(subsystem ? os_log_create(subsystem, "minichromium_logging")
                          : OS_LOG_DEFAULT) {}
  ~OSLog() {
    if (os_log_ != OS_LOG_DEFAULT) {
      os_release(os_log_);
    }
  }
  os_log_t get() const { return os_log_; }

 private:
  os_log_t os_log_;
};

os_log_type_t LogSeverityToOSLogType(cr::logging::LogSeverity severity) {
  switch (severity) {
    case cr::logging::kLogInfo:
      return OS_LOG_TYPE_INFO;
    case cr::logging::kLogWarning:
      return OS_LOG_TYPE_DEFAULT;
    case cr::logging::kLogError:
      return OS_LOG_TYPE_ERROR;
    case cr::logging::kLogFatal:
      return OS_LOG_TYPE_FAULT;
    default:
      return severity < 0 ? OS_LOG_TYPE_DEBUG : OS_LOG_TYPE_DEFAULT;
  }
}

#endif

}  // namespace

namespace cr {
namespace logging {
namespace internal {

bool OutputSystemDebugLog(LogSeverity severity, const std::string& message) {
  if (!LogToSystem())
    return false;

  CFBundleRef main_bundle = CFBundleGetMainBundle();
  CFStringRef main_bundle_id_cf =
      main_bundle ? CFBundleGetIdentifier(main_bundle) : nullptr;
  std::string main_bundle_id =
      main_bundle_id_cf ? CFStringRefToUTF8(main_bundle_id_cf)
                        : std::string("");
#if defined(USE_APPLE_SYSTEM_LOG)
  ASLClient asl_client(main_bundle_id.empty() ? main_bundle_id
                                              : "com.apple.console");
  ASLMessage asl_message;

  // By default, messages are only readable by the admin group. Explicitly
  // make them readable by the user generating the messages.
  char euid_string[12];
  snprintf(euid_string, cr::size(euid_string), "%d", geteuid());
  asl_set(asl_message.get(), ASL_KEY_READ_UID, euid_string);
  asl_set(asl_message.get(),
          ASL_KEY_LEVEL,
          LogSeverityToASLLevelString(severity));
  asl_set(asl_message.get(), ASL_KEY_MSG, message.c_str());
  asl_send(asl_client.get(), asl_message.get());
#else
  OSLog log((main_bundle_id.empty() ? nullptr : main_bundle_id.c_str());
  os_log_with_type(log.get(),
                   LogSeverityToOSLogType(severity),
                   "%{public}s",
                   message.c_str());
#endif
  return true;
}

}  // namespace internal
}  // namespace logging
}  // namespace cr
