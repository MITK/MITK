/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabelSelectionTool.h"
#include "mitkToolManager.h"

#include <usGetModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, LabelSelectionTool, "Selection tool");
}

mitk::LabelSelectionTool::LabelSelectionTool() : SegTool2D("LabelSelectionTool")
{
}

mitk::LabelSelectionTool::~LabelSelectionTool()
{
}

us::ModuleResource mitk::LabelSelectionTool::GetIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("Selection.svg");
}

const char** mitk::LabelSelectionTool::GetXPM() const
{
  return nullptr;
}

const char* mitk::LabelSelectionTool::GetName() const
{
  return "Selection";
}

void mitk::LabelSelectionTool::Activated()
{
  Superclass::Activated();
  m_HighlightGuard.SetHighlightedLabels({});
  m_HighlightGuard.SetSegmentationNode(this->GetWorkingDataNode());
}

void mitk::LabelSelectionTool::Deactivated()
{
  m_HighlightGuard.SetSegmentationNode(nullptr);
  m_HighlightGuard.SetHighlightedLabels({});
  Superclass::Deactivated();
}

void mitk::LabelSelectionTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("Move", OnMouseMoved);
  CONNECT_FUNCTION("Release", OnMouseReleased);
}

mitk::MultiLabelSegmentation::LabelValueVectorType mitk::LabelSelectionTool::GetIndicatedLabels() const
{
  return m_HighlightGuard.GetHighlightedLabels();
}

void mitk::LabelSelectionTool::UpdateLabels()
{
  const auto segmentation = this->GetWorkingData();

  std::optional<MultiLabelSegmentation::GroupIndexType> group;

  if (m_CheckOnlyActiveGroup)
  {
    group = segmentation->GetGroupIndexOfLabel(segmentation->GetActiveLabel()->GetValue());
  }

  auto newLabels = segmentation->GetLabelValuesByCoordinates(m_LastCheckedPoint,
    segmentation->GetTimeGeometry()->TimePointToTimeStep(this->GetLastTimePointTriggered()),
    group);

  bool labelsChanged = newLabels != m_HighlightGuard.GetHighlightedLabels();

  m_HighlightGuard.SetHighlightedLabels(newLabels);

  if (labelsChanged)
    this->IndicatedLabelsChanged.Send();
}

void mitk::LabelSelectionTool::OnMouseMoved(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (!positionEvent)
    return;

  const auto segmentation = this->GetWorkingData();
  auto point = positionEvent->GetPositionInWorld();
  Point3D index;
  segmentation->GetGeometry()->WorldToIndex(point, index);

  if (index == m_LastCheckedIndex)
    return;

  m_LastCheckedIndex = index;
  m_LastCheckedPoint = point;

  this->UpdateLabels();
}

void mitk::LabelSelectionTool::OnMouseReleased(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (!positionEvent)
    return;

  const auto labels = m_HighlightGuard.GetHighlightedLabels();
  if (!labels.empty())
  {

    this->GetToolManager()->SetActiveWorkingLabel( labels.front() );
  }
}
