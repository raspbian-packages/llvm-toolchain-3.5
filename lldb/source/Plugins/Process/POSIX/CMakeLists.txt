set(LLVM_NO_RTTI 1)

include_directories(.)
include_directories(../Linux)
include_directories(../Utility)

add_lldb_library(lldbPluginProcessPOSIX
  POSIXStopInfo.cpp
  POSIXThread.cpp
  ProcessMessage.cpp
  ProcessPOSIX.cpp
  ProcessPOSIXLog.cpp
  RegisterContextPOSIXProcessMonitor_mips64.cpp
  RegisterContextPOSIXProcessMonitor_x86.cpp
  )
