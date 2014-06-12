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

#include "mitkFillRegionTool.h"

#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, FillRegionTool, "Fill tool");
}

mitk::FillRegionTool::FillRegionTool()
:SetRegionTool()
{
}

mitk::FillRegionTool::~FillRegionTool()
{
}

void mitk::FillRegionTool::ConnectActionsAndFunctions()
{
  Superclass::ConnectActionsAndFunctions();
  CONNECT_FUNCTION( "InvertLogic", OnInvertLogic );
}

const char** mitk::FillRegionTool::GetXPM() const
{
  return NULL;
}

us::ModuleResource mitk::FillRegionTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Fill_48x48.png");
  return resource;
}

us::ModuleResource mitk::FillRegionTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Fill_Cursor_32x32.png");
  return resource;
}

const char* mitk::FillRegionTool::GetName() const
{
  return "Fill";
}

bool mitk::FillRegionTool::OnMousePressed (StateMachineAction*, InteractionEvent* interactionEvent)
{
  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
  assert (workingImage);

  m_PaintingPixelValue = workingImage->GetActiveLabel()->GetPixelValue();
  const mitk::Color& color = workingImage->GetActiveLabel()->GetColor();
  this->SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );

  return Superclass::OnMousePressed(NULL, interactionEvent);
}

bool mitk::FillRegionTool::OnInvertLogic(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(interactionEvent) < 1.0 ) return false;

  m_LogicInverted = !m_LogicInverted;

  if (m_LogicInverted)
  {
    LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
    assert (workingImage);
    m_PaintingPixelValue = workingImage->GetActiveLabel()->GetPixelValue();
    const mitk::Color& color = workingImage->GetActiveLabel()->GetColor();
    FeedbackContourTool::SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );
  }
  else
  {
    m_PaintingPixelValue = 0;
    FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
  }

  return true;
}
