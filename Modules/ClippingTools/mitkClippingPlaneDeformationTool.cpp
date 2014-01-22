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
#include "mitkClippingPlaneDeformationTool.h"
#include "mitkClippingPlaneDeformationTool.xpm"

#include "mitkBaseRenderer.h"
#include "usModule.h"
#include "usModuleContext.h"
#include "usGetModuleContext.h"
#include "usModuleRegistry.h"
#include "mitkGlobalInteraction.h"
#include "mitkRenderingManager.h"
#include "mitkToolManager.h"

namespace mitk{
  MITK_TOOL_MACRO(ClippingTools_EXPORT, ClippingPlaneDeformationTool, "Clipping Tool Deformation");
}

mitk::ClippingPlaneDeformationTool::ClippingPlaneDeformationTool()
: Tool( "global" )
{
  m_SurfaceInteractor = mitk::SurfaceDeformationDataInteractor3D::New();
}

mitk::ClippingPlaneDeformationTool::~ClippingPlaneDeformationTool()
{
}

const char** mitk::ClippingPlaneDeformationTool::GetXPM() const
{
  return mitkClippingPlaneDeformationTool_xpm;
}

const char* mitk::ClippingPlaneDeformationTool::GetName() const
{
  return "Deformation";
}

const char* mitk::ClippingPlaneDeformationTool::GetGroup() const
{
  return "ClippingTool";
}

void mitk::ClippingPlaneDeformationTool::Activated()
{
    Superclass::Activated();

    //check if the Clipping plane is changed.
    if(m_ClippingPlaneNode != m_ToolManager->GetWorkingData(0))
    {
      m_ClippingPlaneNode = m_ToolManager->GetWorkingData(0);
      m_ClippingPlaneNode->SetDataInteractor(NULL);
    }

    m_SurfaceInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    m_SurfaceInteractor->SetEventConfig("AffineDeformationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    m_SurfaceInteractor->SetDataNode(m_ClippingPlaneNode);

}

void mitk::ClippingPlaneDeformationTool::Deactivated()
{
  Superclass::Deactivated();

  m_SurfaceInteractor->SetDataNode(NULL);
}
