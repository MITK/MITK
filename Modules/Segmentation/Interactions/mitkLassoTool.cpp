/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//#include <mitkContourModelUtils.h>
#include <mitkLassoTool.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>

#include <type_traits>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, LassoTool, "Lasso tool");
}

mitk::LassoTool::LassoTool() : EditableContourTool()
{
}

mitk::LassoTool::~LassoTool()
{}

void mitk::LassoTool::ConnectActionsAndFunctions()
{
  mitk::EditableContourTool::ConnectActionsAndFunctions();
  CONNECT_FUNCTION("MovePoint", OnMouseMoved);
}

const char **mitk::LassoTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::LassoTool::GetIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("Lasso.svg");
}

us::ModuleResource mitk::LassoTool::GetCursorIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("Lasso_Cursor.svg");
}

const char *mitk::LassoTool::GetName() const
{
  return "Lasso";
}

void mitk::LassoTool::FinishTool() 
{
  auto contourInteractor = mitk::ContourModelInteractor::New();
  contourInteractor->SetDataNode(m_ContourNode);
  contourInteractor->LoadStateMachine("ContourModelModificationInteractor.xml", us::GetModuleContext()->GetModule());
  contourInteractor->SetEventConfig("ContourModelModificationConfig.xml", us::GetModuleContext()->GetModule());
  contourInteractor->SetRestrictedArea(this->m_CurrentRestrictedArea);

  this->m_ContourInteractor = contourInteractor;
  m_ContourNode->SetDataInteractor(m_ContourInteractor.GetPointer());
}
