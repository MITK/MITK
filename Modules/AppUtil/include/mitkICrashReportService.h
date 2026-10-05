/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkICrashReportService_h
#define mitkICrashReportService_h

#include <MitkAppUtilExports.h>

#include <mitkCrashDumpFacility.h>
#include <mitkServiceInterface.h>

#include <vector>

class QWidget;

namespace mitk
{
  /**
   * \brief Hands crash dumps to a report flow.
   *
   * Registered by the report feature. The crash-dump UI (next-start dialog,
   * diagnostic data manager, "Capture diagnostics" result) offers
   * "File report..." only while an implementation is registered. The dumps
   * stay where they are; what the report flow copies, sends or deletes is its
   * own decision and must be visible to the user.
   *
   * Before a dump leaves the computer, the flow must have the user confirm
   * explicitly that they are permitted to share it, i.e. that sharing complies
   * with the data-protection and other rules that apply to the data handled
   * in that session. The local crash-dump UI only advises; that confirmation
   * is what puts the decision on record with the sender.
   */
  class MITKAPPUTIL_EXPORT ICrashReportService
  {
  public:
    virtual ~ICrashReportService();

    /** \brief Start the report flow for \p dumps (non-empty).
     *
     *  Called on the UI thread. \p parent is a widget to parent the flow's
     *  windows to; it may be null, and a flow that outlives this call must
     *  not rely on it staying alive. Long-running work must not block the
     *  UI thread: the UI-freeze watchdog would take it for a freeze. */
    virtual void FileReport(const std::vector<CrashDumpInfo>& dumps, QWidget* parent) = 0;
  };

  /** \brief Whether a report service is registered. */
  MITKAPPUTIL_EXPORT bool IsCrashReportServiceAvailable();

  /** \brief Start the registered service's report flow for \p dumps.
   *
   *  Returns false, doing nothing, if \p dumps is empty or no service is
   *  registered. The service is held only for the duration of the call; a
   *  flow that outlives it is the service's own business. */
  MITKAPPUTIL_EXPORT bool FileCrashReport(const std::vector<CrashDumpInfo>& dumps, QWidget* parent);
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::ICrashReportService, "org.mitk.ICrashReportService")

#endif
