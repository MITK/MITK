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

#include "mitkFillRegionTool.xpm"

#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, FillRegionTool, "Fill tool");
}

mitk::FillRegionTool::FillRegionTool()
:SetRegionTool()
{
  CONNECT_ACTION( 49014, OnInvertLogic );
}

mitk::FillRegionTool::~FillRegionTool()
{
}

const char** mitk::FillRegionTool::GetXPM() const
{
  return mitkFillRegionTool_xpm;
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

bool mitk::FillRegionTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
  assert (workingImage);

  m_PaintingPixelValue = workingImage->GetActiveLabelIndex();
  const mitk::Color& color = workingImage->GetActiveLabelColor();
  this->SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );

  return Superclass::OnMousePressed(action, stateEvent);
}

bool mitk::FillRegionTool::OnInvertLogic(Action* action, const StateEvent* stateEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  m_LogicInverted = !m_LogicInverted;

  if (m_LogicInverted)
  {
    LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
    assert (workingImage);
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
