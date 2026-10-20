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

#include <cstdint>
#include <string>

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

    /**
     * \brief Return how much physical RAM is available for new allocations, in bytes.
     *
     * Includes memory the system can reclaim without swapping, like file caches.
     * On Linux, this is MemAvailable from /proc/meminfo.
     * On Windows, this is the available physical memory (the "Available" value in the task manager).
     * On macOS, this is the sum of free and inactive pages.
     *
     * \return The available physical memory in bytes, or 0 if the value could not be determined.
     */
    MITKCORE_EXPORT size_t GetAvailableSizeOfPhysicalRam();

    /**
     * \brief Formats a size in bytes for people, e.g. "512 B", "4.0 KB" or "1.3 GB".
     *
     * The units are powers of 1024, named B, KB, MB, GB and TB as most user
     * interfaces do, with one decimal above bytes. The format does not depend
     * on the global locale, which suits log messages and exceptions. Qt user
     * interfaces use QLocale::formattedDataSize() instead, which follows the
     * locale of the user.
     */
    MITKCORE_EXPORT std::string FormatSize(std::uint64_t bytes);
  }
}

#endif
