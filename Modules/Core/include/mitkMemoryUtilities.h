/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMemoryUtilities_h
#define mitkMemoryUtilities_h

#include <MitkCoreExports.h>
#include <itkMacro.h>

namespace mitk
{
  namespace MemoryUtilities
  {
    /**
     * Returns the memory usage of the current process in bytes.
     * On Linux, this refers to the virtual memory allocated by
     * the process (the VIRT column in top).
     * On Windows, this refers to the size in bytes of the working
     * set pages (the "Speicherauslastung" column in the task manager).
     */
    MITKCORE_EXPORT size_t GetProcessMemoryUsage();

    /**
     * Returns the total size of physical memory in bytes
     */
    MITKCORE_EXPORT size_t GetTotalSizeOfPhysicalRam();
  }
}

#endif
