/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCreateMultiLabelPresetAction.h"

#include <mitkLabelSetImage.h>
#include <mitkLabelSetIOHelper.h>

#include <QFileDialog>
#include <QMessageBox>

void QmitkCreateMultiLabelPresetAction::Run(const QList<mitk::DataNode::Pointer> &selectedNodes)
{
  for (const auto &node : selectedNodes)
  {
    if (node.IsNull())
      continue;

    mitk::LabelSetImage::Pointer image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());

    if (image.IsNull())
      continue;

    const auto filename = QFileDialog::getSaveFileName(nullptr, QStringLiteral("Save LabelSet Preset"),
      QString(), QStringLiteral("LabelSet Preset (*.lsetp)")).toStdString();

    if (filename.empty())
      continue;

    if(!mitk::LabelSetIOHelper::SaveLabelSetImagePreset(filename, image))
    {
      QMessageBox::critical(nullptr, QStringLiteral("Save LabelSetImage Preset"),
        QString("Could not save \"%1\" as preset.").arg(QString::fromStdString(node->GetName())));
      
      continue;
    }
  }
}

void QmitkCreateMultiLabelPresetAction::SetDataStorage(mitk::DataStorage*)
{
}

void QmitkCreateMultiLabelPresetAction::SetFunctionality(berry::QtViewPart*)
{
}

void QmitkCreateMultiLabelPresetAction::SetSmoothed(bool)
{
}

void QmitkCreateMultiLabelPresetAction::SetDecimated(bool)
{
}
