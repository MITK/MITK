/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkConvertMaskToLabelAction.h"

#include "mitkRenderingManager.h"
#include "mitkLabelSetImage.h"
#include "mitkToolManagerProvider.h"

//needed for qApp
#include <qcoreapplication.h>

QmitkConvertMaskToLabelAction::QmitkConvertMaskToLabelAction()
{
}

QmitkConvertMaskToLabelAction::~QmitkConvertMaskToLabelAction()
{
}

void QmitkConvertMaskToLabelAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  mitk::ToolManager::Pointer toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(toolManager);

  mitk::DataNode* workingNode = toolManager->GetWorkingData(0);
  if (!workingNode)
  {
    MITK_INFO << "There is no available segmentation. Please load or create one before using this tool.";
    return;
  }

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  assert(workingImage);

  foreach ( mitk::DataNode::Pointer maskNode, selectedNodes )
  {
    if (maskNode)
    {
      mitk::Image* mask = dynamic_cast<mitk::Image*>(maskNode->GetData() );
      if (!mask) continue;

      std::string name = maskNode->GetName();
      mitk::Color color;
      mitk::ColorProperty::Pointer colorProp;
      maskNode->GetProperty(colorProp,"color");
      if (colorProp.IsNull()) continue;
      color = colorProp->GetValue();
      workingImage->GetLabelSet()->AddLabel(name,color);
      //workingImage->AddLabelEvent.Send();

      try
      {
        workingImage->MaskStamp( mask, false );
      }
      catch ( mitk::Exception& e )
      {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        return;
      }

      maskNode->SetVisibility(false);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else
    {
      MITK_INFO << "   a nullptr node was selected";
    }
  }
}

void QmitkConvertMaskToLabelAction::SetSmoothed(bool /*smoothed*/)
{
 //not needed
}

void QmitkConvertMaskToLabelAction::SetDecimated(bool /*decimated*/)
{
  //not needed
}

void QmitkConvertMaskToLabelAction::SetDataStorage(mitk::DataStorage* /*dataStorage*/)
{
  //not needed
}

void QmitkConvertMaskToLabelAction::SetFunctionality(berry::QtViewPart* /*functionality*/)
{
  //not needed
}
