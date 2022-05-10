/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCreateMultiLabelSegmentationAction.h"

#include "mitkLabelSetImage.h"
#include "mitkLabelSetImageHelper.h"

#include <QmitkStaticDynamicSegmentationDialog.h>

#include "QMessageBox"

QmitkCreateMultiLabelSegmentationAction::QmitkCreateMultiLabelSegmentationAction()
{
}

QmitkCreateMultiLabelSegmentationAction::~QmitkCreateMultiLabelSegmentationAction()
{
}

void QmitkCreateMultiLabelSegmentationAction::Run(const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  if (m_DataStorage.IsNull())
  {
    auto message = tr("Data storage not set.");
    MITK_ERROR << message;
    QMessageBox::warning(nullptr, "New segmentation", message);
    return;
  }

  for (const auto& referenceNode : selectedNodes)
  {
    if (referenceNode.IsNull())
    {
      continue;
    }

    mitk::Image::ConstPointer  referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
    if (referenceImage.IsNull())
    {
      MITK_WARN << "Could not create multi label segmentation for non-image node.";
      continue;
    }

    if (referenceImage->GetDimension() <= 1)
    {
      MITK_WARN << "Segmentation is currently not supported for 2D images.";
      continue;
    }

    auto segTemplateImage = referenceImage;
    if (referenceImage->GetDimension() > 3)
    {
      QmitkStaticDynamicSegmentationDialog dialog(nullptr);
      dialog.SetReferenceImage(referenceImage.GetPointer());
      dialog.exec();
      segTemplateImage = dialog.GetSegmentationTemplate();
    }

    mitk::DataNode::Pointer newSegmentationNode;
    try
    {
      newSegmentationNode = mitk::LabelSetImageHelper::CreateNewSegmentationNode(referenceNode, segTemplateImage);
    }
    catch (mitk::Exception& e)
    {
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      QMessageBox::warning(nullptr, "New segmentation", "Could not create a new segmentation.");
      return;
    }

    auto newLabelSetImage = dynamic_cast<mitk::LabelSetImage*>(newSegmentationNode->GetData());
    if (nullptr == newLabelSetImage)
    {
      // something went wrong
      return;
    }

    mitk::Label::Pointer newLabel = mitk::LabelSetImageHelper::CreateNewLabel(newLabelSetImage);
    newLabelSetImage->GetActiveLabelSet()->AddLabel(newLabel);

    if (!m_DataStorage->Exists(newSegmentationNode))
    {
      m_DataStorage->Add(newSegmentationNode, referenceNode);
    }
  }
}

void QmitkCreateMultiLabelSegmentationAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCreateMultiLabelSegmentationAction::SetFunctionality(berry::QtViewPart*)
{
  //not needed
}

void QmitkCreateMultiLabelSegmentationAction::SetSmoothed(bool)
{
  //not needed
}

void QmitkCreateMultiLabelSegmentationAction::SetDecimated(bool)
{
  //not needed
}
