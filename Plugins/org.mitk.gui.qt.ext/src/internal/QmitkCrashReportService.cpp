/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCrashReportService.h"

#include "QmitkFeedbackInput.h"

void QmitkCrashReportService::FileReport(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent)
{
  // Collected before the dialog exists, so that no screenshot shows it.
  QmitkRunFeedbackDialog(QmitkCollectCrashReportInput(dumps), parent);
}
