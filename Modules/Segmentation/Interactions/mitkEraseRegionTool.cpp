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

#include "mitkEraseRegionTool.h"

#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"
#include "mitkStateMachineAction.h"
#include "mitkInteractionEvent.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, EraseRegionTool, "Erase tool");
}

mitk::EraseRegionTool::EraseRegionTool()
:SetRegionTool()
{
}

mitk::EraseRegionTool::~EraseRegionTool()
{
}

void mitk::EraseRegionTool::ConnectActionsAndFunctions()
{
  Superclass::ConnectActionsAndFunctions();
  CONNECT_FUNCTION( "InvertLogic", OnInvertLogic );
}

const char** mitk::EraseRegionTool::GetXPM() const
{
  return NULL; //mitkEraseRegionTool_xpm;
}

us::ModuleResource mitk::EraseRegionTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Erase_48x48.png");
  return resource;
}

us::ModuleResource mitk::EraseRegionTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Erase_Cursor_32x32.png");
  return resource;
}

const char* mitk::EraseRegionTool::GetName() const
{
  return "Erase";
}

bool mitk::EraseRegionTool::OnMousePressed (StateMachineAction*, InteractionEvent* interactionEvent)
{
  m_PaintingPixelValue = 0;
  FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );

  return Superclass::OnMousePressed(NULL, interactionEvent);
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to the active label
  and viceversa.
*/
bool mitk::EraseRegionTool::OnInvertLogic(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(interactionEvent) < 1.0 ) return false;

  m_LogicInverted = !m_LogicInverted;

  if (m_LogicInverted)
  {
    LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
    assert(workingImage);
    m_PaintingPixelValue = workingImage->GetActiveLabelIndex();
    const mitk::Color& color = workingImage->GetActiveLabelColor();
    FeedbackContourTool::SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );
  }
  else
  {
    m_PaintingPixelValue = 0;
    FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
  }

  return true;
}
