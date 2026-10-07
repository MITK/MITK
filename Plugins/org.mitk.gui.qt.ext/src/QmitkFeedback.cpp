/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFeedback.h"

#include "internal/QmitkFeedbackInput.h"

void QmitkFeedback::OpenDialog(QWidget* parent)
{
  // Collected before the dialog exists, so that no screenshot shows it.
#ifdef MITK_HAS_CRASHHANDLING
  // Saved dumps are offered for the user to pick, since a report started
  // here may still be about a crash.
  auto input = QmitkCollectCrashReportInput(mitk::CrashDumpFacility::ListAllDumps());
  input.dumpsIncluded = false;
#else
  const auto input = QmitkCollectFeedbackInput();
#endif

  QmitkRunFeedbackDialog(input, parent);
}
