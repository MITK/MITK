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

#include "mitkAddContourTool.h"
#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"
#include "mitkAddContourTool.xpm"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, AddContourTool, "Add tool");
}

mitk::AddContourTool::AddContourTool()
:ContourTool()
{
  CONNECT_ACTION( 49014, OnInvertLogic );
}

mitk::AddContourTool::~AddContourTool()
{
}

bool mitk::AddContourTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  assert (workingNode);

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(workingNode->GetData());
  assert (workingImage);

  int activeLayer = workingImage->GetActiveLayer();
  m_PaintingPixelValue = workingImage->GetActiveLabelIndex( activeLayer );
  const mitk::Color& color = workingImage->GetActiveLabelColor( activeLayer );
  this->SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );

  return Superclass::OnMousePressed(action, stateEvent);
}

const char** mitk::AddContourTool::GetXPM() const
{
  return mitkAddContourTool_xpm;
}

us::ModuleResource mitk::AddContourTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Add_48x48.png");
  return resource;
}

us::ModuleResource mitk::AddContourTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Add_Cursor_32x32.png");
  return resource;
}

const char* mitk::AddContourTool::GetName() const
{
  return "Add";
}

bool mitk::AddContourTool::OnInvertLogic(Action* action, const StateEvent* stateEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;
  m_LogicInverted = !m_LogicInverted;
  if (m_LogicInverted)
  {
    m_PaintingPixelValue = 0;
    FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
  }
  else
  {
    DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
    assert (workingNode);
    LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(workingNode->GetData());
    assert (workingImage);
    int activeLayer = workingImage->GetActiveLayer();
    m_PaintingPixelValue = workingImage->GetActiveLabelIndex( activeLayer );
    const mitk::Color& color = workingImage->GetActiveLabelColor( activeLayer );
    FeedbackContourTool::SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );
  }
  return true;
}
