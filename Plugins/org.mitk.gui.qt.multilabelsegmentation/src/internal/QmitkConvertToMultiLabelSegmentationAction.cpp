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
  foreach ( mitk::DataNode::Pointer referenceNode, selectedNodes )
  {
    if (referenceNode.IsNotNull())
    {
      mitk::Image::Pointer referenceImage = dynamic_cast<mitk::Image*>( referenceNode->GetData() );
      if (referenceImage.IsNull()) return;

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

void QmitkConvertToMultiLabelSegmentationAction::SetSmoothed(bool smoothed)
{
  //not needed
}

void QmitkConvertToMultiLabelSegmentationAction::SetDecimated(bool decimated)
{
  //not needed
}
