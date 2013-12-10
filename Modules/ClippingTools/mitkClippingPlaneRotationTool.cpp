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
#include "mitkClippingPlaneRotationTool.h"
#include "mitkClippingPlaneRotationTool.xpm"

#include "mitkBaseRenderer.h"
#include "usModule.h"
#include "usModuleContext.h"
#include "usGetModuleContext.h"
#include "usModuleRegistry.h"
#include "mitkRenderingManager.h"
#include "mitkToolManager.h"


namespace mitk
{
  MITK_TOOL_MACRO(ClippingTools_EXPORT, ClippingPlaneRotationTool, "Clipping Tool Rotation");
}

mitk::ClippingPlaneRotationTool::ClippingPlaneRotationTool()
: Tool( "global" )
{
  m_AffineDataInteractor = mitk::AffineDataInteractor3D::New();
}

mitk::ClippingPlaneRotationTool::~ClippingPlaneRotationTool()
{
}

const char** mitk::ClippingPlaneRotationTool::GetXPM() const
{
  return mitkClippingPlaneRotationTool_xpm;
}

const char* mitk::ClippingPlaneRotationTool::GetName() const
{
  return "Rotation";
}

const char* mitk::ClippingPlaneRotationTool::GetGroup() const
{
  return "ClippingTool";
}

void mitk::ClippingPlaneRotationTool::Activated()
{
  Superclass::Activated();

  //check if the Clipping plane is changed.
  if(m_ClippingPlaneNode != m_ToolManager->GetWorkingData(0))
  {
    m_ClippingPlaneNode = m_ToolManager->GetWorkingData(0);
    m_ClippingPlaneNode->SetDataInteractor(NULL);
  }

  m_AffineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkExt"));
  m_AffineDataInteractor->SetEventConfig("AffineRotationConfig.xml", us::ModuleRegistry::GetModule("MitkExt"));
  m_AffineDataInteractor->SetDataNode(m_ClippingPlaneNode);
}

void mitk::ClippingPlaneRotationTool::Deactivated()
{
  Superclass::Deactivated();
  m_AffineDataInteractor->SetDataNode(NULL);
}
