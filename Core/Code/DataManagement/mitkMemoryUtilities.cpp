/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkMemoryUtilities.h"

#include <stdio.h>
#if _MSC_VER || __MINGW32__
  #include <windows.h>
  #include <psapi.h>
#elif defined(__APPLE__)
  #include <mach/task.h>
  #include <mach/mach_init.h>
  #include <mach/mach_host.h>
  #include <sys/sysctl.h>
#else
  #include <sys/sysinfo.h>
  #include <unistd.h>
#endif


/**
 * Returns the memory usage of the current process in bytes.
 * On linux, this refers to the virtual memory allocated by
 * the process (the VIRT column in top).
 * On windows, this refery to the size in bytes of the working
 * set pages (the "Speicherauslastung" column in the task manager).
 */
size_t mitk::MemoryUtilities::GetProcessMemoryUsage()
{
#if _MSC_VER || __MINGW32__
  size_t size = 0;
  DWORD pid = GetCurrentProcessId();
  PROCESS_MEMORY_COUNTERS pmc;
  HANDLE hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid );
  if ( hProcess == NULL )
      return 0;
  if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
  {
    size = pmc.WorkingSetSize;
  }
  CloseHandle( hProcess );
  return size;
#elif defined(__APPLE__)
  struct task_basic_info t_info;
  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
  task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
  size_t size = t_info.virtual_size;
  return size;
#else
  int size, res, shared, text, sharedLibs, stack, dirtyPages;
  if ( ! ReadStatmFromProcFS( &size, &res, &shared, &text, &sharedLibs, &stack, &dirtyPages ) )
    return (size_t) size * getpagesize();
  else
    return 0;
#endif
  return 0;
}


/**
 * Returns the total size of phyiscal memory in bytes
 */
size_t mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam()
{
#if _MSC_VER || __MINGW32__
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof (statex);
  GlobalMemoryStatusEx (&statex);
  return (size_t) statex.ullTotalPhys;
#elif defined(__APPLE__)
  int mib[2];
  int64_t physical_memory;
  mib[0] = CTL_HW;
  mib[1] = HW_MEMSIZE;
  size_t length = sizeof(int64_t);
  sysctl(mib, 2, &physical_memory, &length, NULL, 0);
  return physical_memory;
#else
  struct sysinfo info;
  if ( ! sysinfo( &info ) )
    return info.totalram * info.mem_unit;
  else
    return 0;
#endif
}

#ifndef _MSC_VER
#ifndef __APPLE__
int mitk::MemoryUtilities::ReadStatmFromProcFS( int* size, int* res, int* shared, int* text, int* sharedLibs, int* stack, int* dirtyPages )
{
  int ret = 0;
  FILE* f;
  f = fopen( "/proc/self/statm", "r" );
  if( f ) {
    size_t ignored = fscanf( f, "%d %d %d %d %d %d %d", size, res, shared, text, sharedLibs, stack, dirtyPages );
                ++ignored;
    fclose( f );
  } else {
    ret = -1;
  }
  return ret;
}
#endif
#endif


