/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLoadMultiLabelPresetAction.h"

#include <mitkLabelSetImage.h>
#include <mitkLabelSetIOHelper.h>

#include <QFileDialog>

void QmitkLoadMultiLabelPresetAction::Run(const QList<mitk::DataNode::Pointer> &selectedNodes)
{
  const auto filename = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Load LabelSet Preset"),
    QString(), QStringLiteral("LabelSet Preset (*.lsetp)")).toStdString();

  if (filename.empty())
    return;

  for (const auto &node : selectedNodes)
  {
    if (node.IsNull())
      continue;

    mitk::LabelSetImage::Pointer image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());

    if (image.IsNull())
      continue;

    mitk::LabelSetIOHelper::LoadLabelSetImagePreset(filename, image);
  }
}

void QmitkLoadMultiLabelPresetAction::SetDataStorage(mitk::DataStorage*)
{
}

void QmitkLoadMultiLabelPresetAction::SetFunctionality(berry::QtViewPart*)
{
}

void QmitkLoadMultiLabelPresetAction::SetSmoothed(bool)
{
}

void QmitkLoadMultiLabelPresetAction::SetDecimated(bool)
{
}
