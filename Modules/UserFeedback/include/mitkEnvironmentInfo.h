/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEnvironmentInfo_h
#define mitkEnvironmentInfo_h

#include <MitkUserFeedbackExports.h>

#include <string>
#include <utility>
#include <vector>

namespace mitk
{
  /**
   * \brief Non-sensitive facts about the running application and host,
   *        gathered for a user-reviewed feedback report.
   *
   * Deliberately omits the machine hostname and any data-derived strings. All
   * fields are plain strings so the struct carries no Qt/ITK/VTK types across
   * its boundary. gpuRenderer is left empty by CollectEnvironmentInfo(); the
   * workbench layer fills it from the live render window, because an offscreen
   * GL context can report a different GPU than VTK actually renders with on
   * multi-GPU hosts.
   */
  struct MITKUSERFEEDBACK_EXPORT EnvironmentInfo
  {
    std::string mitkVersion;             /**< MITK_VERSION_STRING. */
    std::string mitkRevision;            /**< MITK_REVISION (full git hash). */
    std::string mitkRevisionDescription; /**< MITK_REVISION_DESC (git describe). */
    std::string buildType;               /**< "Debug" or "Release", derived from NDEBUG. */
    std::string qtVersion;               /**< QT_VERSION_STR. */
    std::string vtkVersion;              /**< VTK_VERSION. */
    std::string itkVersion;              /**< ITK_VERSION_MAJOR.MINOR.PATCH. */
    std::string os;                      /**< QSysInfo::prettyProductName(). */
    std::string kernelType;              /**< QSysInfo::kernelType(). */
    std::string kernelVersion;           /**< QSysInfo::kernelVersion(). */
    std::string cpuArchitecture;         /**< QSysInfo::currentCpuArchitecture(). */
    std::string uiLocale;                /**< QLocale::system().name(). */
    std::string gpuRenderer;             /**< GL renderer; filled by the UI layer, else empty. */
  };

  /** \brief Collect every field except gpuRenderer. Never reads the hostname. */
  MITKUSERFEEDBACK_EXPORT EnvironmentInfo CollectEnvironmentInfo();

  /** \brief Fixed-order (label, value) pairs for every non-empty field. */
  MITKUSERFEEDBACK_EXPORT std::vector<std::pair<std::string, std::string>> ToLabeledFields(const EnvironmentInfo& info);
}

#endif
