/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "QmitkConvertToMultiLabelSegmentationAction.h"

#include "mitkLabelSetImage.h"
#include "mitkRenderingManager.h"

//needed for qApp
#include <qcoreapplication.h>

QmitkConvertToMultiLabelSegmentationAction::QmitkConvertToMultiLabelSegmentationAction()
{
}

QmitkConvertToMultiLabelSegmentationAction::~QmitkConvertToMultiLabelSegmentationAction()
{
}

void QmitkConvertToMultiLabelSegmentationAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  for (auto referenceNode : selectedNodes)
  {
    if (referenceNode.IsNotNull())
    {
      mitk::Image* referenceImage = dynamic_cast<mitk::Image*>(referenceNode->GetData());
      if (nullptr == referenceImage)
      {
        MITK_WARN << "Could not convert to multi label segmentation for non-image node - skipping action.";
        continue;
      }

      mitk::LabelSetImage::Pointer lsImage = mitk::LabelSetImage::New();
      try
      {
        lsImage->InitializeByLabeledImage(referenceImage);
      }
      catch (mitk::Exception &e)
      {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        return;
      }
      if (m_DataStorage.IsNotNull())
      {
        mitk::DataNode::Pointer newNode = mitk::DataNode::New();
        std::string newName = referenceNode->GetName();
        newName += "-labels";
        newNode->SetName(newName);
        newNode->SetData(lsImage);
        m_DataStorage->Add(newNode,referenceNode);
      }
      lsImage->Modified();
    }
  }
}

void QmitkConvertToMultiLabelSegmentationAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkConvertToMultiLabelSegmentationAction::SetFunctionality(berry::QtViewPart* /*functionality*/)
{
  //not needed
}

void QmitkConvertToMultiLabelSegmentationAction::SetSmoothed(bool)
{
  //not needed
}

void QmitkConvertToMultiLabelSegmentationAction::SetDecimated(bool)
{
  //not needed
}
