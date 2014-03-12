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
#include "mitkClippingPlaneTranslationTool.h"
#include "mitkClippingPlaneTranslationTool.xpm"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkToolManager.h"

// us
#include "usModule.h"
#include "usModuleContext.h"
#include "usGetModuleContext.h"
#include "usModuleRegistry.h"

namespace mitk
{
  MITK_TOOL_MACRO(MitkClippingTools_EXPORT, ClippingPlaneTranslationTool, "Clipping Tool Translation");
}

mitk::ClippingPlaneTranslationTool::ClippingPlaneTranslationTool()
: Tool( "" )
{
  m_AffineDataInteractor = mitk::AffineDataInteractor3D::New();
}

mitk::ClippingPlaneTranslationTool::~ClippingPlaneTranslationTool()
{
}

const char** mitk::ClippingPlaneTranslationTool::GetXPM() const
{
  return mitkClippingPlaneTranslationTool_xpm;
}

const char* mitk::ClippingPlaneTranslationTool::GetName() const
{
  return "Translation";
}

const char* mitk::ClippingPlaneTranslationTool::GetGroup() const
{
  return "ClippingTool";
}

void mitk::ClippingPlaneTranslationTool::Activated()
{
    Superclass::Activated();

    //check if the Clipping plane is changed.
    if(m_ClippingPlaneNode != m_ToolManager->GetWorkingData(0))
    {
      m_ClippingPlaneNode = m_ToolManager->GetWorkingData(0);
      m_ClippingPlaneNode->SetDataInteractor(NULL);
    }

    m_AffineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    m_AffineDataInteractor->SetEventConfig("AffineTranslationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    m_AffineDataInteractor->SetDataNode(m_ClippingPlaneNode);
}

void mitk::ClippingPlaneTranslationTool::Deactivated()
{
  Superclass::Deactivated();
  m_AffineDataInteractor->SetDataNode(NULL);
}
