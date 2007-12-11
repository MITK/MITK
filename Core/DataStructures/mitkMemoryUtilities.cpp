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
#else
  struct sysinfo info;
  if ( ! sysinfo( &info ) )
    return info.totalram * info.mem_unit;
  else
    return 0;
#endif 
}

#ifndef _MSC_VER 
int mitk::MemoryUtilities::ReadStatmFromProcFS( int* size, int* res, int* shared, int* text, int* sharedLibs, int* stack, int* dirtyPages ) 
{
	int ret = 0;
	FILE* f;
	f = fopen( "/proc/self/statm", "r" );
	if( f ) {
		fscanf( f, "%d %d %d %d %d %d %d", size, res, shared, text, sharedLibs, stack, dirtyPages );
		fclose( f );
	} else {
		ret = -1;
	}
	return ret;
}
#endif 


