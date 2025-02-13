/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitknnInteractiveTool.h"

#include <mitkDisplayActionEventBroadcast.h>
#include <mitkPointSetDataInteractor.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, nnInteractiveTool, "nnInteractive");
}

namespace
{
  void SetCommonPointsetProperties(mitk::DataNode* node, float r, float g, float b)
  {
    node->SetColor(r, g, b);
    node->SetColor(r, g, b, nullptr, "selectedcolor");
    node->SetProperty("Pointset.2D.shape", mitk::PointSetShapeProperty::New(mitk::PointSetShapeProperty::CIRCLE));
    node->SetIntProperty("point line width", 2);
    node->SetIntProperty("Pointset.2D.resolution", 64);
    node->SetFloatProperty("Pointset.2D.distance to plane", 0.1f);
    node->SetBoolProperty("Pointset.2D.keep shape when selected", true);
    node->SetBoolProperty("Pointset.2D.selected.show contour", true);
    node->SetBoolProperty("Pointset.2D.fill shape", true);
    node->SetBoolProperty("helper object", true);
  }
}

mitk::nnInteractiveTool::nnInteractiveTool()
  : m_PromptType(PromptType::Positive),
    m_Tools {
      Tool::Point,
      Tool::Box,
      Tool::Scribble,
      Tool::Lasso }
{
}

mitk::nnInteractiveTool::~nnInteractiveTool()
{
}

const std::array<mitk::nnInteractiveTool::Tool, 4>& mitk::nnInteractiveTool::GetTools() const
{
  return m_Tools;
}

void mitk::nnInteractiveTool::EnableInteraction(Tool tool, PromptType promptType)
{
  if (m_ActiveTool.has_value())
  {
    if (m_ActiveTool == tool)
    {
      if (m_PromptType == promptType)
        return;
    }

    this->DisableInteraction();
  }

  switch (tool)
  {
    case Tool::Point:
    {
      auto node = this->GetPointSetNode(promptType);

      if (node->GetDataInteractor().IsNull())
      {
        auto interactor = mitk::PointSetDataInteractor::New();
        interactor->LoadStateMachine("PointSet.xml");
        interactor->SetEventConfig("PointSetConfigLMB.xml");
        interactor->EnableMovement(false);
        interactor->SetDataNode(node);
      }

      break;
    }

    case Tool::Box:
      break;

    case Tool::Scribble:
      break;

    case Tool::Lasso:
      break;

    default:
      break;
  }

  m_ActiveTool = tool;
  m_PromptType = promptType;

  this->BlockLMBDisplayInteraction();
}

void mitk::nnInteractiveTool::DisableInteraction()
{
  if (!m_ActiveTool.has_value())
    return;

  this->UnblockLMBDisplayInteraction();

  switch (m_ActiveTool.value())
  {
    case Tool::Point:
    {
      auto node = this->GetPointSetNode(m_PromptType);
      node->SetDataInteractor(nullptr);
      node->GetDataAs<PointSet>()->ClearSelection();

      RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

    case Tool::Box:
      break;

    case Tool::Scribble:
      break;

    case Tool::Lasso:
      break;

    default:
      break;
  }

  m_ActiveTool.reset();
}

void mitk::nnInteractiveTool::BlockLMBDisplayInteraction()
{
  if (!m_EventConfigBackup.empty())
    return;

  for (const auto& eventObserverReference : us::GetModuleContext()->GetServiceReferences<InteractionEventObserver>())
  {
    auto eventObserver = us::GetModuleContext()->GetService(eventObserverReference);
    auto eventBroadcast = dynamic_cast<DisplayActionEventBroadcast*>(eventObserver);

    if (eventBroadcast != nullptr)
    {
      m_EventConfigBackup.emplace_back(eventObserverReference, eventBroadcast->GetEventConfig());
      eventBroadcast->AddEventConfig("DisplayConfigBlockLMB.xml");
    }
  }
}

void mitk::nnInteractiveTool::UnblockLMBDisplayInteraction()
{
  for (const auto& [eventObserverReference, eventConfig] : m_EventConfigBackup)
  {
    if (eventObserverReference)
    {
      auto eventObserver = us::GetModuleContext()->GetService<InteractionEventObserver>(eventObserverReference);

      if (eventObserver)
        static_cast<DisplayActionEventBroadcast*>(eventObserver)->SetEventConfig(eventConfig);
    }
  }

  m_EventConfigBackup.clear();
}

mitk::DataNode* mitk::nnInteractiveTool::GetPointSetNode(PromptType promptType) const
{
  if (promptType == PromptType::Positive)
    return m_PositivePointsNode;

  return m_NegativePointsNode;
}

void mitk::nnInteractiveTool::Activated()
{
  Superclass::Activated();

  auto positivePoints = PointSet::New();
  m_PositivePointsNode = DataNode::New();

  SetCommonPointsetProperties(m_PositivePointsNode, 0.125f, 0.5f, 0.125f);
  m_PositivePointsNode->SetName("nnInteractive positive points");
  m_PositivePointsNode->SetData(positivePoints);

  this->GetDataStorage()->Add(m_PositivePointsNode, this->GetToolManager()->GetReferenceData(0));

  auto negativePoints = PointSet::New();
  m_NegativePointsNode = DataNode::New();

  SetCommonPointsetProperties(m_NegativePointsNode, 0.625f, 0.125f, 0.125f);
  m_NegativePointsNode->SetName("nnInteractive negative points");
  m_NegativePointsNode->SetData(negativePoints);

  this->GetDataStorage()->Add(m_NegativePointsNode, this->GetToolManager()->GetReferenceData(0));
}

void mitk::nnInteractiveTool::Deactivated()
{
  this->GetDataStorage()->Remove(m_NegativePointsNode);
  m_NegativePointsNode = nullptr;

  this->GetDataStorage()->Remove(m_PositivePointsNode);
  m_PositivePointsNode = nullptr;

  Superclass::Deactivated();
}

void mitk::nnInteractiveTool::DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep)
{
}

const char* mitk::nnInteractiveTool::GetName() const
{
  return "nnInteractive";
}

const char** mitk::nnInteractiveTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::nnInteractiveTool::GetIconResource() const
{
  auto module = us::GetModuleContext()->GetModule();
  auto resource = module->GetResource("AI.svg");
  return resource;
}
