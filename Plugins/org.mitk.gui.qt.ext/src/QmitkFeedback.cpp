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
  if (parent == nullptr)
    parent = QmitkGetActiveWorkbenchWindowWidget();

  // Collected before the dialog exists, so that no screenshot shows it.
  QmitkFeedbackDialog dialog(QmitkCollectFeedbackInput(), parent);
  dialog.exec();
}
