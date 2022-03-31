/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSaveMultiLabelPresetAction.h"

#include <mitkLabelSetImage.h>
#include <mitkLabelSetIOHelper.h>

#include <QFileDialog>
#include <QMessageBox>

void QmitkSaveMultiLabelPresetAction::Run(const QList<mitk::DataNode::Pointer> &selectedNodes)
{
  for (const auto &node : selectedNodes)
  {
    if (node.IsNull())
      continue;

    mitk::LabelSetImage::Pointer image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());

    if (image.IsNull())
      continue;

    const auto filename = QFileDialog::getSaveFileName(nullptr, QStringLiteral("Save Label Set Preset"),
      QString(), QStringLiteral("Label set preset (*.lsetp)")).toStdString();

    if (filename.empty())
      continue;

    if(!mitk::LabelSetIOHelper::SaveLabelSetImagePreset(filename, image))
    {
      QMessageBox::critical(nullptr, QStringLiteral("Save Label Set Preset"),
        QString("Could not save \"%1\" as label set preset.").arg(QString::fromStdString(node->GetName())));
      
      continue;
    }
  }
}

void QmitkSaveMultiLabelPresetAction::SetDataStorage(mitk::DataStorage*)
{
}

void QmitkSaveMultiLabelPresetAction::SetFunctionality(berry::QtViewPart*)
{
}

void QmitkSaveMultiLabelPresetAction::SetSmoothed(bool)
{
}

void QmitkSaveMultiLabelPresetAction::SetDecimated(bool)
{
}
