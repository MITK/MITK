/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkCreateSurfaceTool.h"

#include "mitkCreateSurfaceTool.xpm"

#include "mitkShowSegmentationAsSurface.h"
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkToolManager.h"

#include "itkCommand.h"

namespace mitk {
  MITK_TOOL_MACRO(MitkExt_EXPORT, CreateSurfaceTool, "Surface creation tool");
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
      surfaceFilter->AddObserver(mitk::ResultAvailable(), goodCommand);
      itk::SimpleMemberCommand<CreateSurfaceTool>::Pointer badCommand = itk::SimpleMemberCommand<CreateSurfaceTool>::New();
      badCommand->SetCallbackFunction(this, &CreateSurfaceTool::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(mitk::ProcessingError(), badCommand);

      DataNode::Pointer nodepointer = node;
      surfaceFilter->SetPointerParameter("Input", image);
      surfaceFilter->SetPointerParameter("Group node", nodepointer);
      surfaceFilter->SetParameter("Show result", true );
      surfaceFilter->SetParameter("Sync visibility", false );
      surfaceFilter->SetDataStorage( *m_ToolManager->GetDataStorage() );
      
      ProgressBar::GetInstance()->AddStepsToDo(1);
      StatusBar::GetInstance()->DisplayText("Surface creation started in background...");
      surfaceFilter->StartAlgorithm();
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

