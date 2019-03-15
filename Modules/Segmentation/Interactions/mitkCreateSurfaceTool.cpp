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

#include "mitkCreateSurfaceTool.h"

#include "mitkCreateSurfaceTool.xpm"

#include <mitkImage.h>
#include <mitkImageCast.h>
#include "mitkShowSegmentationAsSurface.h"
#include <mitkSurface.h>
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkToolManager.h"

#include "itkCommand.h"

namespace mitk {
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, CreateSurfaceTool, "Surface creation tool");
}

mitk::CreateSurfaceTool::CreateSurfaceTool()
{
}

mitk::CreateSurfaceTool::~CreateSurfaceTool()
{
}

const char** mitk::CreateSurfaceTool::GetXPM() const
{
  return mitkCreateSurfaceTool_xpm;
}

const char* mitk::CreateSurfaceTool::GetName() const
{
  return "Surface";
}

std::string mitk::CreateSurfaceTool::GetErrorMessage()
{
  return "No surfaces created for these segmentations:";
}

bool mitk::CreateSurfaceTool::ProcessOneWorkingData( DataNode* node )
{
  if (node)
  {
    Image::Pointer image = dynamic_cast<Image*>( node->GetData() );
    if (image.IsNull()) return false;

    try
    {
      mitk::ShowSegmentationAsSurface::Pointer surfaceFilter = mitk::ShowSegmentationAsSurface::New();

      // attach observer to get notified about result
      itk::SimpleMemberCommand<CreateSurfaceTool>::Pointer goodCommand = itk::SimpleMemberCommand<CreateSurfaceTool>::New();
      goodCommand->SetCallbackFunction(this, &CreateSurfaceTool::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(itk::EndEvent(), goodCommand);
      itk::SimpleMemberCommand<CreateSurfaceTool>::Pointer badCommand = itk::SimpleMemberCommand<CreateSurfaceTool>::New();
      badCommand->SetCallbackFunction(this, &CreateSurfaceTool::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(itk::AbortEvent(), badCommand);

      DataNode::Pointer nodepointer = node;
      
      ShowSegmentationAsSurface::InputImageType::Pointer itkImage;
      CastToItkImage(image, itkImage);

      surfaceFilter->SetInput(itkImage);

      ProgressBar::GetInstance()->AddStepsToDo(1);
      StatusBar::GetInstance()->DisplayText("Surface creation started in background...");
      surfaceFilter->Update();

      DataNode::Pointer resultNode = DataNode::New();
      Surface::Pointer surf = Surface::New();

      surf->SetVtkPolyData(surfaceFilter->GetOutput());
      resultNode->SetData(surf);

      m_ToolManager->GetDataStorage()->Add(resultNode, nodepointer);
    }
    catch(...)
    {
      return false;
    }
  }

  return true;
}

void mitk::CreateSurfaceTool::OnSurfaceCalculationDone()
{
  ProgressBar::GetInstance()->Progress();
  m_ToolManager->NewNodesGenerated();
}

