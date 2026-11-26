/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrashReportService_h
#define QmitkCrashReportService_h

#include <mitkICrashReportService.h>

/**
 * \brief Files crash dumps through the feedback dialog.
 *
 * The flow is modal and complete when FileReport() returns: the dialog runs
 * a nested event loop, which keeps the UI-freeze watchdog's heartbeat alive,
 * and writes the bundle off the UI thread. Dumps the user asked to delete
 * are deleted after the dialog has closed, and an open diagnostic data
 * manager is refreshed after the deletion.
 */
class QmitkCrashReportService : public mitk::ICrashReportService
{
public:
  void FileReport(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent) override;
};

#endif
