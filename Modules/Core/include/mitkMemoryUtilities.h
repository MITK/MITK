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
  /** \brief Utility functions for querying system and process memory information. */
  namespace MemoryUtilities
  {
    /**
     * \brief Return the memory usage of the current process in bytes.
     *
     * On Linux, this refers to the resident memory minus shared pages
     * (as reported by the GNOME System Monitor).
     * On Windows, this refers to the size in bytes of the working
     * set pages (the "Speicherauslastung" column in the task manager).
     * On macOS, this refers to the resident size minus reusable memory.
     *
     * \return The process memory usage in bytes, or 0 if the value could not be determined.
     */
    MITKCORE_EXPORT size_t GetProcessMemoryUsage();

    /**
     * \brief Return the total size of physical RAM in bytes.
     *
     * \return The total physical memory in bytes, or 0 if the value could not be determined.
     */
    MITKCORE_EXPORT size_t GetTotalSizeOfPhysicalRam();
  }
}

#endif
