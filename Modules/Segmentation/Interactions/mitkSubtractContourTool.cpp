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

#include "mitkSubtractContourTool.h"
#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"
#include "mitkLookupTableProperty.h"
#include "mitkSubtractContourTool.xpm"

// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include <mitkGetModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, SubtractContourTool, "Subtract tool");
}

mitk::SubtractContourTool::SubtractContourTool()
:ContourTool()
{
    CONNECT_ACTION( 49014, OnInvertLogic );
}

mitk::SubtractContourTool::~SubtractContourTool()
{
}

bool mitk::SubtractContourTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
    m_PaintingPixelValue = 0;
    FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );

    return Superclass::OnMousePressed(action, stateEvent);
}

const char** mitk::SubtractContourTool::GetXPM() const
{
  return mitkSubtractContourTool_xpm;
}

mitk::ModuleResource mitk::SubtractContourTool::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Subtract_48x48.png");
  return resource;
}

mitk::ModuleResource mitk::SubtractContourTool::GetCursorIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Subtract_Cursor_32x32.png");
  return resource;
}

const char* mitk::SubtractContourTool::GetName() const
{
  return "Subtract";
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to the active label
  and viceversa.
*/
bool mitk::SubtractContourTool::OnInvertLogic(Action* action, const StateEvent* stateEvent)
{
    if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

    m_LogicInverted = !m_LogicInverted;

    if (m_LogicInverted)
    {
        DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
        if (!workingNode) return false;
        LabelSetImage* image = dynamic_cast<LabelSetImage*>(workingNode->GetData());
        m_PaintingPixelValue = image->GetActiveLabelIndex();
        const mitk::Color& color = image->GetActiveLabelColor();
        FeedbackContourTool::SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );
    }
    else
    {
        m_PaintingPixelValue = 0;
        FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
    }

  return true;
}
