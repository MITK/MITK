/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkMemoryUtilities.h"

#include <stdio.h>
#ifdef _MSC_VER 
  #include <windows.h>
  #include <psapi.h>
#elif defined(__APPLE__)
  #include <mach/task.h>
  #include <mach/mach_init.h>
  #include <mach/mach_host.h>
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
#ifdef _MSC_VER 
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
#ifdef _MSC_VER 
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof (statex);
  GlobalMemoryStatusEx (&statex);
  return (size_t) statex.ullTotalPhys;
#elif defined(__APPLE__)
  kern_return_t kr;
  host_basic_info_data_t hostinfo;
  int count = HOST_BASIC_INFO_COUNT;
  kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostinfo, (mach_msg_type_number_t*)&count);
  if(kr == KERN_SUCCESS)
    return (size_t)hostinfo.memory_size;
  else
    return 0;
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


