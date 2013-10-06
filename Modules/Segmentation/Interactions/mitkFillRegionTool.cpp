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
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include <mitkGetModuleContext.h>

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

mitk::ModuleResource mitk::FillRegionTool::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Fill_48x48.png");
  return resource;
}

mitk::ModuleResource mitk::FillRegionTool::GetCursorIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Fill_Cursor_32x32.png");
  return resource;
}

const char* mitk::FillRegionTool::GetName() const
{
  return "Fill";
}

bool mitk::FillRegionTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  assert (workingNode);

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(workingNode->GetData());
  assert (workingImage);

  unsigned int activeLayer = workingImage->GetActiveLayer();
  m_PaintingPixelValue = workingImage->GetActiveLabelIndex(activeLayer);
  const mitk::Color& color = workingImage->GetActiveLabelColor(activeLayer);
  this->SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );

  return Superclass::OnMousePressed(action, stateEvent);
}

bool mitk::FillRegionTool::OnInvertLogic(Action* action, const StateEvent* stateEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  m_LogicInverted = !m_LogicInverted;

  if (m_LogicInverted)
  {
    DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
    assert (workingNode);
    LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(workingNode->GetData());
    assert (workingImage);
    unsigned int activeLayer = workingImage->GetActiveLayer();
    m_PaintingPixelValue = workingImage->GetActiveLabelIndex(activeLayer);
    const mitk::Color& color = workingImage->GetActiveLabelColor(activeLayer);
    FeedbackContourTool::SetFeedbackContourColor( color.GetRed(), color.GetGreen(), color.GetBlue() );
  }
  else
  {
    m_PaintingPixelValue = 0;
    FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
  }

  return true;
}
