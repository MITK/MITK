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

#include "mitkStateMachineAction.h"
#include "mitkInteractionEvent.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, AddContourTool, "Add tool");
}

mitk::AddContourTool::AddContourTool()
:ContourTool()
{

}

mitk::AddContourTool::~AddContourTool()
{
}

void mitk::AddContourTool::ConnectActionsAndFunctions()
{
  Superclass::ConnectActionsAndFunctions();
  CONNECT_FUNCTION( "InvertLogic", OnInvertLogic );
}

bool mitk::AddContourTool::OnMousePressed (StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (this->CanHandleEvent(interactionEvent) > 0.0)
  {

    LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
    assert (workingImage);

    m_PaintingPixelValue = workingImage->GetActiveLabelIndex();
    const mitk::Color& color = workingImage->GetActiveLabelColor();
    this->SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );

    return Superclass::OnMousePressed(NULL, interactionEvent);
  }
  else
    return false;
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

bool mitk::AddContourTool::OnInvertLogic(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(interactionEvent) < 1.0 ) return false;
  m_LogicInverted = !m_LogicInverted;
  if (m_LogicInverted)
  {
    m_PaintingPixelValue = 0;
    FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
  }
  else
  {
    LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
    assert (workingImage);
    m_PaintingPixelValue = workingImage->GetActiveLabelIndex();
    const mitk::Color& color = workingImage->GetActiveLabelColor();
    FeedbackContourTool::SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );
  }
  return true;
}
