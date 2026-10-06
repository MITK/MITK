/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCrashReportService.h"

#include "QmitkFeedbackDialog.h"
#include "QmitkFeedbackInput.h"

#include <QMessageBox>
#include <QStringList>

void QmitkCrashReportService::FileReport(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent)
{
  if (parent == nullptr)
    parent = QmitkGetActiveWorkbenchWindowWidget();

  std::vector<std::filesystem::path> dumpsToDelete;

  {
    // Collected before the dialog exists, so that no screenshot shows it.
    QmitkFeedbackDialog dialog(QmitkCollectCrashReportInput(dumps), parent);
    dialog.exec();
    dumpsToDelete = dialog.GetDumpsToDelete();
  }

  QStringList failed;

  for (const auto& dump : dumpsToDelete)
  {
    if (!mitk::CrashDumpFacility::DeleteDump(dump))
      failed << QString::fromStdU16String(dump.filename().u16string());
  }

  if (!failed.isEmpty())
  {
    QMessageBox::warning(parent, "Report a Problem / Send Feedback",
      "These crash dumps could not be deleted:\n\n" + failed.join('\n') +
      "\n\nYou can delete them later under Help > Diagnostic Data.");
  }
}
