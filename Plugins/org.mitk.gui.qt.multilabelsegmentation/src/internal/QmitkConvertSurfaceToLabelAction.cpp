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
#include "QmitkConvertSurfaceToLabelAction.h"

#include "mitkRenderingManager.h"
#include "mitkLabelSetImage.h"
#include "mitkToolManagerProvider.h"
#include <mitkSurface.h>

//needed for qApp
#include <qcoreapplication.h>
#include <QApplication>

QmitkConvertSurfaceToLabelAction::QmitkConvertSurfaceToLabelAction()
{
}

QmitkConvertSurfaceToLabelAction::~QmitkConvertSurfaceToLabelAction()
{
}

void QmitkConvertSurfaceToLabelAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
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

  foreach ( mitk::DataNode::Pointer surfaceNode, selectedNodes )
  {
    if (surfaceNode)
    {
      mitk::Surface* surface = dynamic_cast<mitk::Surface*>(surfaceNode->GetData() );
      if (!surface) continue;

      std::string name = surfaceNode->GetName();
      mitk::Color color;
      mitk::ColorProperty::Pointer colorProp;
      surfaceNode->GetProperty(colorProp,"color");
      if (colorProp.IsNull()) continue;
      color = colorProp->GetValue();
      workingImage->GetLabelSet()->AddLabel(name,color);
      //workingImage->AddLabelEvent.Send();

      try
      {
        QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
//        workingImage->SurfaceStamp( surface, false );
        QApplication::restoreOverrideCursor();
      }
      catch ( mitk::Exception& e )
      {
        QApplication::restoreOverrideCursor();
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        return;
      }

      surfaceNode->SetVisibility(false);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else
    {
      MITK_INFO << "   a NULL node was selected";
    }
  }
}

void QmitkConvertSurfaceToLabelAction::SetSmoothed(bool /*smoothed*/)
{
 //not needed
}

void QmitkConvertSurfaceToLabelAction::SetDecimated(bool /*decimated*/)
{
  //not needed
}

void QmitkConvertSurfaceToLabelAction::SetDataStorage(mitk::DataStorage* /*dataStorage*/)
{
  //not needed
}

void QmitkConvertSurfaceToLabelAction::SetFunctionality(berry::QtViewPart* /*functionality*/)
{
  //not needed
}
