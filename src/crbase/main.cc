// Copyright (c) 2012 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

#include "crbase/logging/logging.h"
#include "crbase/utils/debug/immediate_crash.h"
#include "crbase/utils/template/class_method_detector.h"
#include "crbase/numerics/safe_math.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/string_number_conversions.h"
#include "crbase/containers/span.h"
#include "crbase/atomic/atomicops.h"
#include "crbase/containers/optional.h"
#include "crbase/functional/callback_list.h"
#include "crbase/functional/cancelable_callback.h"
#include "crbase/files/file_path.h"
#include "crbase/system/cpu_info.h"
#include "crbase/synchronization/lock.h"
#include "crbase/time/time.h"
#include "crbase/files/file_enumerator.h"
#include "crbase/threading/thread_local.h"
#include "crbase/message_loop/message_loop.h"
#include "crbase/message_loop/run_loop.h"
#include "crbase/threading/thread_task_runner_handle.h"
#include "crbase/threading/task/task_runner_util.h"
#include "crbase/timer/timer.h"
#include "crbase/at_exit.h"
#include "crbase/compiler/build_config.h"

namespace {

const char* BoolToString(bool b) {
  return b ? ": true" : ": false";
}

}  // namespace

void TestLog(int i) {
  CR_LOG(Info) << "Test Log:" << i;
}

void PrintCpuInfo() {
  auto& cpu_info = cr::CPUInfo::GetInstanceNoAllocation();

  CR_LOG(Info) << "**********CPU Information**********";
  CR_LOG(Info) << "* vendor_name: " << cpu_info.vendor_name();
  CR_LOG(Info) << "* cpu_brand: " << cpu_info.cpu_brand();

#if defined(MINI_CHROMIUM_ARCH_CPU_X86_FAMILY)
  CR_LOG(Info) << "* has_mmx" << BoolToString(cpu_info.has_mmx());
  CR_LOG(Info) << "* has_sse" << BoolToString(cpu_info.has_sse());
  CR_LOG(Info) << "* has_sse2" << BoolToString(cpu_info.has_sse2());
  CR_LOG(Info) << "* has_sse3" << BoolToString(cpu_info.has_sse3());
  CR_LOG(Info) << "* has_ssse3" << BoolToString(cpu_info.has_ssse3());
  CR_LOG(Info) << "* has_sse41" << BoolToString(cpu_info.has_sse41());
  CR_LOG(Info) << "* has_sse42" << BoolToString(cpu_info.has_sse42());
  CR_LOG(Info) << "* has_popcnt" << BoolToString(cpu_info.has_popcnt());
  CR_LOG(Info) << "* has_avx" << BoolToString(cpu_info.has_avx());
  CR_LOG(Info) << "* has_fma3" << BoolToString(cpu_info.has_fma3());
  CR_LOG(Info) << "* has_avx2" << BoolToString(cpu_info.has_avx2());
  CR_LOG(Info) << "* has_avx512_f" << BoolToString(cpu_info.has_avx512_f());
  CR_LOG(Info) << "* has_avx512_bw" << BoolToString(cpu_info.has_avx512_bw());
  CR_LOG(Info) << "* has_avx512_vnni" << BoolToString(cpu_info.has_avx512_vnni());
  CR_LOG(Info) << "* has_pclmul" << BoolToString(cpu_info.has_pclmul());
  CR_LOG(Info) << "* has_pku" << BoolToString(cpu_info.has_pku());
#endif

#if defined(MINI_CHROMIUM_ARCH_CPU_ARM_FAMILY)
  CR_LOG(Info) << "* has_bti" << BoolToString(cpu_info.has_bti());
  CR_LOG(Info) << "* has_mte" << BoolToString(cpu_info.has_mte());
#endif

  CR_LOG(Info) << "* has_aesni" << BoolToString(cpu_info.has_aesni());
  CR_LOG(Info) << "* has_non_stop_time_stamp_counter"
    << BoolToString(cpu_info.has_non_stop_time_stamp_counter());
  CR_LOG(Info) << "* is_running_in_vm" << BoolToString(cpu_info.is_running_in_vm());
}

int main(int argc, char* argv[]) {
  cr::logging::LoggingConfig& config = CR_DEFAULT_LOGGING_CONFIG;
  config.logging_dest = cr::logging::LOG_TO_STDERR /* | cr::logging::LOG_TO_FILE*/;
  cr::logging::InitializeConfig(config);

  cr::AtExitManager at_exit;

  if (argc)
    CR_LOG(Info) << "executable file:" << argv[0];

  CR_LOG(Info) << "hello mini_chromium!";

  cr::StringPiece16 view16(u"123456");
  cr::WStringPiece view_wide(L"123456");

  CR_LOG(Info) << view16.length() << ":" << view16.substr(5).length();
  CR_LOG(Info) << view_wide.length() << ":"  << view_wide.substr(2).length();

  std::u16string str16 = view16.substr(2);
  cr::MakeStringPiece16(str16.begin(), str16.end());

  CR_LOG(Info) << str16.length() << " : aa";

  int i1 = 5, i2 = 6, i3 = 100;

  auto clamp_r1 = cr::ClampAdd(i1, i2);
  auto clamp_r2 = cr::ClampSub(i2, i1);
  auto clamp_r3 = cr::ClampMul(i1, i3);
  auto clamp_r4 = cr::ClampDiv(i3, i1);
  CR_LOG(Info) << clamp_r1;
  CR_LOG(Info) << clamp_r2;
  CR_LOG(Info) << clamp_r3;
  CR_LOG(Info) << clamp_r4;

  float f1 = 0.4f, f2 = 0.5f, f3 = 0.6f, f4 = 1.4f;
  auto out_f1 = cr::ClampRound(f1);
  auto out_f2 = cr::ClampRound(f2);
  auto out_f3 = cr::ClampRound(f3);
  auto out_f4 = cr::ClampRound(f4);

  CR_LOG(Info) << out_f1;
  CR_LOG(Info) << out_f2;
  CR_LOG(Info) << out_f3;
  CR_LOG(Info) << out_f4;

  std::vector<uint16_t> vec = {1,2,3,4,5,6,7,8,9};

  auto span = cr::MakeSpan(vec).as_writable_bytes();
  CR_LOG(Info) << "span size:" << span.size();

  std::string str_f_num = cr::NumberToString(-14414545678.1);
  CR_LOG(Info) << "str_f_num:" << str_f_num;

  cr::optional<bool> opt_r = cr::MakeOptional<bool>(true);
  CR_LOG(Info) << ((opt_r.has_value() ? opt_r.value() : false) ? "true" : "false");

  cr::CancelableOnceClosure ts_fun(cr::BindOnce(&TestLog, 1));

  auto func1 = ts_fun.callback();
  auto func2 = ts_fun.callback();

  std::move(func2).Run();
  ts_fun.Cancel();
  std::move(func1).Run();

  cr::FilePath sys_path = cr::FilePath::FromUTF8Unsafe("~").AppendASCII("root");
  CR_LOG(Info) << "sys_path:" << sys_path;

  PrintCpuInfo();

  static int locked_a = 1;

  cr::Lock lock;
  {
    cr::AutoLock autolock(lock);
    locked_a++;
  }
  CR_LOG(Info) << "locked_a = " << locked_a;

  CR_LOG(Info) << "NowTime:" << cr::Time::Now();

  {
    CR_LOG(Info) << "*** StartEnumPath*****";
    cr::FileEnumerator file_enum(cr::FilePath::FromUTF8Unsafe("."), false,
                                 cr::FileEnumerator::FILES,
                                 CR_FILE_PATH_LITERAL("*"));
    for (cr::FilePath name = file_enum.Next(); !name.empty();
         name = file_enum.Next())
      CR_LOG(Info) << "EnumPath:" << name;
    CR_LOG(Info) << "*** Ended *****";
  }

  static cr::ThreadLocalBoolean s_tls;
  {
    s_tls.Set(true);
    CR_LOG(Info) << "tls:" << BoolToString(s_tls.Get());
  }

  cr::MessageLoop message_loop(cr::MessageLoop::TYPE_UI);
  cr::RunLoop bottom_loop;
  
  ///
  cr::RepeatingTimer timer;
  timer.Start(CR_FROM_HERE, cr::TimeDelta::FromSeconds(3),
              cr::BindRepeating([]()->void {CR_LOG(Info) << "Message from timer..."; }));

  auto task_runner = cr::ThreadTaskRunnerHandle::Get();
  task_runner->PostTask(CR_FROM_HERE, cr::BindOnce(&TestLog, 101));

  CR_LOG(Info) << "posted messagebox task!";
  bottom_loop.Run();
  getchar();
  return 0;
}
