/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitknnInteractiveTool.h"

#include <mitkDisplayActionEventBroadcast.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkScribbleTool.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlanarRectangle.h>
#include <mitkPointSetDataInteractor.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, nnInteractiveTool, "nnInteractive");
}

const mitk::Color& mitk::nnInteractiveTool::GetColor(PromptType promptType, Intensity intensity)
{
  static auto vibrantPositiveColor = MakeColor(0.0f, 0.694f, 0.047f);
  static auto mutedPositiveColor = MakeColor(0.0f, 0.472f, 0.027f);
  static auto vibrantNegativeColor = MakeColor(0.827f, 0.027f, 0.0f);
  static auto mutedNegativeColor = MakeColor(0.561f, 0.02f, 0.0f);

  if (promptType == PromptType::Positive)
  {
    return intensity == Intensity::Vibrant
      ? vibrantPositiveColor
      : mutedPositiveColor;
  }

  return intensity == Intensity::Vibrant
    ? vibrantNegativeColor
    : mutedNegativeColor;
}

mitk::nnInteractiveTool::nnInteractiveTool()
  : m_PromptType(PromptType::Positive),
    m_Tools {
      Tool::Point,
      Tool::Box,
      Tool::Scribble,
      Tool::Lasso },
    m_ScribbleTool(ScribbleTool::New())
{
}

mitk::nnInteractiveTool::~nnInteractiveTool()
{
}

void mitk::nnInteractiveTool::SetToolManager(ToolManager* toolManager)
{
  Superclass::SetToolManager(toolManager);

  m_ToolManager = ToolManager::New(toolManager->GetDataStorage());
  m_ToolManager->SetReferenceData(toolManager->GetReferenceData(0));

  m_ScribbleTool->InitializeStateMachine();
  m_ScribbleTool->SetToolManager(m_ToolManager);
}

void mitk::nnInteractiveTool::Notify(InteractionEvent* interactionEvent, bool isHandled)
{
  if (!isHandled)
  {
    if (m_ScribbleTool->IsEnabled())
    {
      m_ScribbleTool->HandleEvent(interactionEvent, nullptr);
      return;
    }
  }

  return Superclass::Notify(interactionEvent, isHandled);
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
      m_PointSetInteractor->SetDataNode(this->GetPointSetNode(promptType));
      m_PointSetInteractor->EnableInteraction(true);
      this->BlockLMBDisplayInteraction();
      break;
    }

    case Tool::Box:
      this->AddNewBoxNode(promptType);
      this->BlockLMBDisplayInteraction();
      break;

    case Tool::Scribble:
      this->AddScribbleNode();
      this->SetActiveScribbleLabel(promptType);
      m_ToolManager->SetWorkingData(m_ScribbleNode);
      m_ScribbleTool->Activate(this->GetColor(promptType, Intensity::Vibrant));
      break;

    case Tool::Lasso:
      break;

    default:
      break;
  }

  m_ActiveTool = tool;
  m_PromptType = promptType;
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
      m_PointSetInteractor->EnableInteraction(false);
      m_PointSetInteractor->SetDataNode(nullptr);
      this->GetPointSetNode(m_PromptType)->GetDataAs<PointSet>()->ClearSelection();
      break;
    }

    case Tool::Box:
      this->RemoveNewBoxNode();
      break;

    case Tool::Scribble:
      m_ScribbleTool->Deactivate();
      break;

    case Tool::Lasso:
      break;

    default:
      break;
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
  m_ActiveTool.reset();
}

void mitk::nnInteractiveTool::ResetInteractions()
{
  this->RemoveScribbleNode();
  this->RemoveNewBoxNode();

  for (auto promptType : { PromptType::Positive, PromptType::Negative })
  {
    this->GetPointSetNode(promptType)->GetDataAs<PointSet>()->Clear();

    auto nodes = this->GetBoxNodes(promptType);

    for (const auto& node : nodes)
      this->GetDataStorage()->Remove(node);

    nodes.clear();
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
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

std::vector<mitk::DataNode::Pointer>& mitk::nnInteractiveTool::GetBoxNodes(PromptType promptType)
{
  if (promptType == PromptType::Positive)
    return m_PositiveBoxNodes;

  return m_NegativeBoxNodes;
}

void mitk::nnInteractiveTool::AddNewBoxNode(PromptType promptType)
{
  if (m_NewBoxNode.first.IsNotNull())
    return;

  auto node = DataNode::New();
  node->SetName(this->CreateNodeName("box", promptType));
  node->SetData(PlanarRectangle::New());
  node->SetColor(this->GetColor(promptType, Intensity::Vibrant));
  node->SetBoolProperty("planarfigure.drawname", false);
  node->SetBoolProperty("planarfigure.drawshadow", false);
  node->SetBoolProperty("planarfigure.drawcontrolpoints", false);
  node->SetBoolProperty("planarfigure.hidecontrolpointsduringinteraction", true);
  node->SetBoolProperty("planarfigure.fill", true);
  node->SetBoolProperty("helper object", true);

  m_PlanarFigureInteractor->SetDataNode(node);
  m_PlanarFigureInteractor->EnableInteraction(true);

  auto command = itk::SimpleMemberCommand<Self>::New();
  command->SetCallbackFunction(this, &Self::OnBoxPlaced);
  auto tag = node->GetData()->AddObserver(EndPlacementPlanarFigureEvent(), command);

  this->GetDataStorage()->Add(node, this->GetToolManager()->GetReferenceData(0));

  m_NewBoxNode = std::make_pair(node, tag);
}

void mitk::nnInteractiveTool::OnBoxPlaced()
{
  auto& [node, tag] = m_NewBoxNode;

  node->GetData()->RemoveObserver(tag);
  node->SetColor(this->GetColor(m_PromptType, Intensity::Muted));

  this->GetBoxNodes(m_PromptType).push_back(node);
  node = nullptr;

  this->AddNewBoxNode(m_PromptType);
}

void mitk::nnInteractiveTool::RemoveNewBoxNode()
{
  m_PlanarFigureInteractor->EnableInteraction(false);
  m_PlanarFigureInteractor->SetDataNode(nullptr);

  if (m_NewBoxNode.first.IsNotNull())
  {
    this->GetDataStorage()->Remove(m_NewBoxNode.first);
    m_NewBoxNode.first = nullptr;
  }
}

void mitk::nnInteractiveTool::AddScribbleNode()
{
  if (m_ScribbleNode.IsNotNull())
    return;

  auto referenceNode = this->GetToolManager()->GetReferenceData(0);
  std::string name = this->CreateNodeName("scribble");

  m_ScribbleNode = LabelSetImageHelper::CreateNewSegmentationNode(nullptr, referenceNode->GetDataAs<Image>(), name);
  m_ScribbleNode->SetBoolProperty("helper object", true);

  for (auto promptType : { PromptType::Positive, PromptType::Negative })
    this->AddScribbleLabel(promptType);

  this->GetDataStorage()->Add(m_ScribbleNode, referenceNode);
}

void mitk::nnInteractiveTool::AddScribbleLabel(PromptType promptType)
{
  auto name = this->GetPromptTypeString(promptType);
  const auto& color = this->GetColor(promptType, Intensity::Muted);

  auto label = m_ScribbleNode->GetDataAs<LabelSetImage>()->AddLabel(name, color, 0);
  label->SetLocked(false);

  m_ScribbleLabels[promptType] = label->GetValue();
}

void mitk::nnInteractiveTool::SetActiveScribbleLabel(PromptType promptType)
{
  m_ScribbleNode->GetDataAs<LabelSetImage>()->SetActiveLabel(m_ScribbleLabels[promptType]);
  m_ScribbleNode->SetBoolProperty("labelset.contour.active", false);
}

void mitk::nnInteractiveTool::RemoveScribbleNode()
{
  if (m_ScribbleNode.IsNull())
    return;

  this->GetDataStorage()->Remove(m_ScribbleNode);
  m_ScribbleNode = nullptr;

  m_ScribbleLabels.clear();
}

std::string mitk::nnInteractiveTool::CreateNodeName(const std::string& name, std::optional<PromptType> promptType) const
{
  std::stringstream stream;
  stream << this->GetName();

  if (promptType.has_value())
    stream << ' ' << this->GetPromptTypeString(promptType.value());

  stream << ' ' << name;

  return stream.str();
}

std::string mitk::nnInteractiveTool::GetPromptTypeString(PromptType promptType) const
{
  if (promptType == PromptType::Positive)
    return "positive";

  return "negative";
}

mitk::DataNode::Pointer mitk::nnInteractiveTool::CreatePointSetNode(PromptType promptType) const
{
  auto name = this->CreateNodeName("points", promptType);
  const auto& color = this->GetColor(promptType, Intensity::Muted);

  auto node = mitk::DataNode::New();
  node->SetData(mitk::PointSet::New());
  node->SetName(name);
  node->SetColor(color);
  node->SetColor(color, nullptr, "selectedcolor");
  node->SetProperty("Pointset.2D.shape", mitk::PointSetShapeProperty::New(mitk::PointSetShapeProperty::CIRCLE));
  node->SetIntProperty("point line width", 2);
  node->SetIntProperty("Pointset.2D.resolution", 64);
  node->SetFloatProperty("point 2D size", 10.0f);
  node->SetFloatProperty("Pointset.2D.distance to plane", 0.1f);
  node->SetBoolProperty("Pointset.2D.keep shape when selected", true);
  node->SetBoolProperty("Pointset.2D.selected.show contour", true);
  node->SetBoolProperty("Pointset.2D.fill shape", true);
  node->SetBoolProperty("helper object", true);

  this->GetDataStorage()->Add(node, this->GetToolManager()->GetReferenceData(0));

  return node;
}

void mitk::nnInteractiveTool::CreatePointSetInteractor()
{
  m_PointSetInteractor = mitk::PointSetDataInteractor::New();
  m_PointSetInteractor->LoadStateMachine("PointSet.xml");
  m_PointSetInteractor->SetEventConfig("PointSetConfigLMB.xml");
  m_PointSetInteractor->EnableInteraction(false);
  m_PointSetInteractor->EnableMovement(false);
}

void mitk::nnInteractiveTool::CreatePlanarFigureInteractor()
{
  auto module = us::ModuleRegistry::GetModule("MitkPlanarFigure");

  m_PlanarFigureInteractor = mitk::PlanarFigureInteractor::New();
  m_PlanarFigureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", module);
  m_PlanarFigureInteractor->SetEventConfig("PlanarFigureConfig.xml", module);
  m_PlanarFigureInteractor->EnableInteraction(false);
}

void mitk::nnInteractiveTool::Activated()
{
  Superclass::Activated();

  m_PositivePointsNode = this->CreatePointSetNode(PromptType::Positive);
  m_NegativePointsNode = this->CreatePointSetNode(PromptType::Negative);

  this->CreatePointSetInteractor();
  this->CreatePlanarFigureInteractor();
}

void mitk::nnInteractiveTool::Deactivated()
{
  this->DisableInteraction();
  this->ResetInteractions();

  m_PlanarFigureInteractor = nullptr;
  m_PointSetInteractor = nullptr;

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
