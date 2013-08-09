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
#include "mitkLookupTableProperty.h"
#include "mitkAddContourTool.xpm"

// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include <mitkGetModuleContext.h>

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
    if (!workingNode) return false;
    LabelSetImage* image = dynamic_cast<LabelSetImage*>(workingNode->GetData());
    m_PaintingPixelValue = image->GetActiveLabelIndex();
    const mitk::Color& color = image->GetActiveLabelColor();
    FeedbackContourTool::SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );

    return Superclass::OnMousePressed(action, stateEvent);
}

const char** mitk::AddContourTool::GetXPM() const
{
  return mitkAddContourTool_xpm;
}

mitk::ModuleResource mitk::AddContourTool::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Add_48x48.png");
  return resource;
}

mitk::ModuleResource mitk::AddContourTool::GetCursorIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Add_Cursor_32x32.png");
  return resource;
}

const char* mitk::AddContourTool::GetName() const
{
  return "Add";
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to the active label
  and viceversa.
*/
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
        if (!workingNode) return false;
        LabelSetImage* image = dynamic_cast<LabelSetImage*>(workingNode->GetData());
        m_PaintingPixelValue = image->GetActiveLabelIndex();
        const mitk::Color& color = image->GetActiveLabelColor();
        FeedbackContourTool::SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );
    }

    return true;
}
