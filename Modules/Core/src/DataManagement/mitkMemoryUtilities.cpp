/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMemoryUtilities.h>

#if _MSC_VER
#include <windows.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <mach/task.h>
#include <sys/sysctl.h>
#else
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fstream>
#include <string>
#endif

size_t mitk::MemoryUtilities::GetProcessMemoryUsage()
{
  size_t size = 0;

#if _MSC_VER
  DWORD pid = GetCurrentProcessId();
  PROCESS_MEMORY_COUNTERS pmc;

  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

  if (hProcess != nullptr)
  {
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) != 0)
      size = pmc.WorkingSetSize;

    CloseHandle(hProcess);
  }
#elif defined(__APPLE__)
  task_vm_info taskInfo;
  mach_msg_type_number_t count = TASK_VM_INFO_COUNT;

  if (task_info(mach_task_self(), TASK_VM_INFO, reinterpret_cast<task_info_t>(&taskInfo), &count) == KERN_SUCCESS)
    size = taskInfo.resident_size - taskInfo.reusable; // That's what Chromium reports as process memory
#else
  std::ifstream statm("/proc/self/statm");

  if (statm.is_open())
  {
    size_t resident = 0;
    size_t shared = 0;

    statm >> size >> resident >> shared;

    statm.close();

    if (shared < resident)
      size = resident - shared; // That's what the GNOME System Monitor reports as process memory
  }
#endif

  return size;
}

size_t mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam()
{
#if _MSC_VER
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);

  if (GlobalMemoryStatusEx(&statex) != 0)
    return statex.ullTotalPhys;
#elif defined(__APPLE__)
  int mib[2];
  mib[0] = CTL_HW;
  mib[1] = HW_MEMSIZE;

  int64_t physical_memory;
  size_t length = sizeof(int64_t);

  sysctl(mib, 2, &physical_memory, &length, nullptr, 0);

  return physical_memory;
#else
  struct sysinfo info;

  if (!sysinfo(&info))
    return info.totalram * info.mem_unit;
#endif

  return 0;
}

size_t mitk::MemoryUtilities::GetAvailableSizeOfPhysicalRam()
{
#if _MSC_VER
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);

  if (GlobalMemoryStatusEx(&statex) != 0)
    return statex.ullAvailPhys;
#elif defined(__APPLE__)
  vm_statistics64_data_t statistics;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  vm_size_t pageSize = 0;

  // Every call of mach_host_self() hands out a port right that has to be given back.
  const mach_port_t host = mach_host_self();

  const bool success =
    host_statistics64(host, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&statistics), &count) == KERN_SUCCESS &&
    host_page_size(host, &pageSize) == KERN_SUCCESS;

  mach_port_deallocate(mach_task_self(), host);

  if (success)
    return (static_cast<size_t>(statistics.free_count) + statistics.inactive_count) * pageSize;
#else
  // Unlike sysinfo()'s freeram, MemAvailable counts the page cache the kernel
  // can reclaim, which on a machine that has been running for a while is most
  // of what is actually available.
  std::ifstream meminfo("/proc/meminfo");
  std::string key;
  size_t kiloBytes = 0;

  while (meminfo >> key >> kiloBytes)
  {
    if (key == "MemAvailable:")
      return kiloBytes * 1024;

    meminfo.ignore(256, '\n');
  }
#endif

  return 0;
}
