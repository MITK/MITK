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
#include <mitknnInteractiveLassoTool.h>
#include <mitknnInteractiveScribbleTool.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>
#include <mitkPointSetDataInteractor.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkSegmentationHelper.h>
#include <mitkToolManager.h>
#include <mitkImageReadAccessor.h>
#include <itkEventObject.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, nnInteractiveTool, "nnInteractive");
}

std::string mitk::nnInteractiveTool::GetToolString(Tool tool)
{
  switch (tool)
  {
    case Tool::Box:
      return "box";

    case Tool::Lasso:
      return "lasso";

    case Tool::Point:
      return "point";

    default:
      return "scribble";
  }
}

std::string mitk::nnInteractiveTool::GetPromptTypeString(PromptType promptType)
{
  if (promptType == PromptType::Positive)
    return "positive";

  return "negative";
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
  : m_AutoZoom(true),
    m_MaskImage(nullptr),
    m_PromptType(PromptType::Positive),
    m_Tools {
      Tool::Point,
      Tool::Box,
      Tool::Scribble,
      Tool::Lasso },
    m_ScribbleTool(nnInteractiveScribbleTool::New()),
    m_LassoTool(nnInteractiveLassoTool::New()),
    m_IsSessionReady(false),
    m_LastSetTimePoint(std::numeric_limits<double>::lowest()),
    SegWithPreviewTool(true) // prevents auto-compute across all timesteps
{
  auto scribbleCommand = itk::MemberCommand<Self>::New();
  scribbleCommand->SetCallbackFunction(this, &Self::OnScribbleEvent);
  m_ScribbleTool->AddObserver(ScribbleEvent(), scribbleCommand);

  auto lassoClosedCommand = itk::MemberCommand<Self>::New();
  lassoClosedCommand->SetCallbackFunction(this, &Self::OnLassoEvent);
  m_LassoTool->AddObserver(LassoEvent(), lassoClosedCommand);
  
  this->ResetsToEmptyPreviewOn();
  this->IsTimePointChangeAwareOff();
  this->KeepActiveAfterAcceptOn();
}

bool mitk::nnInteractiveTool::GetAutoZoom() const
{
  return m_AutoZoom;
}

void mitk::nnInteractiveTool::SetAutoZoom(bool autoZoom)
{
  m_AutoZoom = autoZoom;
}

mitk::nnInteractiveTool::~nnInteractiveTool(){}

void mitk::nnInteractiveTool::CleanUpSession()
{
  if (m_PythonContext.IsNotNull() && m_PythonContext->IsVariableExists("session"))
  {
    std::string pyCommand = "session._reset_session()\n"
                            "del session.network\n"
                            "del session\n"
                            "torch.cuda.empty_cache()\n";
    m_PythonContext->ExecuteString(pyCommand);
  }
}

void mitk::nnInteractiveTool::SetToolManager(ToolManager* toolManager)
{
  Superclass::SetToolManager(toolManager);

  m_ToolManager = ToolManager::New(toolManager->GetDataStorage());

  m_ScribbleTool->InitializeStateMachine();
  m_ScribbleTool->SetToolManager(m_ToolManager);

  m_LassoTool->InitializeStateMachine();
  m_LassoTool->SetToolManager(m_ToolManager);
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
    else if (m_LassoTool->IsEnabled())
    {
      m_LassoTool->HandleEvent(interactionEvent, nullptr);
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
      m_PointInteractor->SetDataNode(this->GetPointSetNode(promptType));
      m_PointInteractor->EnableInteraction(true);
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
      m_ToolManager->SetReferenceData(this->GetToolManager()->GetReferenceData(0));
      m_ToolManager->SetWorkingData(m_ScribbleNode);
      m_ScribbleTool->Activate(this->GetColor(promptType, Intensity::Vibrant));
      break;

    case Tool::Lasso:
      this->AddLassoMaskNode(promptType);
      m_ToolManager->SetReferenceData(this->GetToolManager()->GetReferenceData(0));
      m_ToolManager->SetWorkingData(m_LassoMaskNode);
      m_LassoTool->Activate(this->GetColor(promptType, Intensity::Vibrant));
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
      m_PointInteractor->EnableInteraction(false);
      m_PointInteractor->SetDataNode(nullptr);
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
      m_LassoTool->Deactivate();
      this->RemoveLassoMaskNode();
      break;

    default:
      break;
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
  m_ActiveTool.reset();
}

void mitk::nnInteractiveTool::ResetInteractions()
{
  this->RemoveLassoMaskNode();
  this->RemoveScribbleNode();
  this->RemoveNewBoxNode();

  for (auto promptType : { PromptType::Positive, PromptType::Negative })
  {
    this->GetPointSetNode(promptType)->GetDataAs<PointSet>()->Clear();

    auto& nodes = this->GetBoxNodes(promptType);

    for (const auto& node : nodes)
      this->GetDataStorage()->Remove(node);

    nodes.clear();

    nodes = this->GetLassoNodes(promptType);

    for (const auto& node : nodes)
      this->GetDataStorage()->Remove(node);

    nodes.clear();
  }
  this->ResetPreviewContent();

  if (m_PythonContext.IsNotNull() && m_PythonContext->IsVariableExists("session"))
  {
    std::string pyCommand = "if session:\n"
                            "   session.reset_interactions()\n";
    m_PythonContext->ExecuteString(pyCommand);
  }
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

mitk::DataNode::Pointer mitk::nnInteractiveTool::CreatePointSetNode(PromptType promptType)
{
  auto pointSet = PointSet::New();
  auto geometry = static_cast<ProportionalTimeGeometry*>(pointSet->GetTimeGeometry());
  geometry->SetStepDuration(std::numeric_limits<TimePointType>::max());

  auto pointCommand = itk::SimpleMemberCommand<Self>::New();
  pointCommand->SetCallbackFunction(this, &Self::OnPointEvent);
  pointSet->AddObserver(PointSetAddEvent(), pointCommand);

  auto node = DataNode::New();
  node->SetData(pointSet);
  node->SetName(this->CreateNodeName("points", promptType));
  node->SetColor(this->GetColor(promptType, Intensity::Muted));
  node->SetColor(this->GetColor(promptType, Intensity::Vibrant), nullptr, "selectedcolor");
  node->SetProperty("Pointset.2D.shape", PointSetShapeProperty::New(PointSetShapeProperty::CIRCLE));
  node->SetIntProperty("Pointset.2D.resolution", 64);
  node->SetFloatProperty("point 2D size", 5.0f);
  node->SetFloatProperty("Pointset.2D.distance to plane", 0.1f);
  node->SetBoolProperty("Pointset.2D.keep shape when selected", true);
  node->SetBoolProperty("Pointset.2D.fill shape", true);
  node->SetBoolProperty("helper object", true);

  this->GetDataStorage()->Add(node, this->GetToolManager()->GetReferenceData(0));

  return node;
}

bool mitk::nnInteractiveTool::IsNewTimePoint()
{
  const TimePointType currentTimePoint =
    RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  if (m_LastSetTimePoint != currentTimePoint)
  {
    return true;
  }
  return false;
}

void mitk::nnInteractiveTool::OnPointEvent()
{
  if (this->IsNewTimePoint())
  {//set current 3D image vol into the nnInter session
    this->SetImageInSession();
  }
  this->UpdatePreview();
}

mitk::DataNode* mitk::nnInteractiveTool::GetPointSetNode(PromptType promptType) const
{
  if (promptType == PromptType::Positive)
    return m_PositivePointsNode;

  return m_NegativePointsNode;
}

const std::vector<mitk::DataNode::Pointer>& mitk::nnInteractiveTool::GetBoxNodes(PromptType promptType) const
{
  if (promptType == PromptType::Positive)
    return m_PositiveBoxNodes;

  return m_NegativeBoxNodes;
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
  node->SetColor(this->GetColor(promptType, Intensity::Muted));
  node->SetColor(this->GetColor(promptType, Intensity::Vibrant), nullptr, "planarfigure.selected.line.color");
  node->SetSelected(true);
  node->SetFloatProperty("planarfigure.line.width", 3.0f);
  node->SetBoolProperty("planarfigure.drawname", false);
  node->SetBoolProperty("planarfigure.drawshadow", false);
  node->SetBoolProperty("planarfigure.drawcontrolpoints", false);
  node->SetBoolProperty("planarfigure.hidecontrolpointsduringinteraction", true);
  node->SetBoolProperty("planarfigure.fill", true);
  node->SetBoolProperty("helper object", true);

  m_BoxInteractor->SetDataNode(node);
  m_BoxInteractor->EnableInteraction(true);

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
  node->SetSelected(false);

  this->GetBoxNodes(m_PromptType).push_back(node);

  if (this->IsNewTimePoint())
  {//set current 3D image vol into the nnInter session
    this->SetImageInSession();
  }
  this->UpdatePreview();
  node = nullptr;
  this->AddNewBoxNode(m_PromptType);
}

void mitk::nnInteractiveTool::RemoveNewBoxNode()
{
  m_BoxInteractor->EnableInteraction(false);
  m_BoxInteractor->SetDataNode(nullptr);

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
  auto referenceImage = referenceNode->GetDataAs<Image>();
  auto templateImage = SegmentationHelper::GetStaticSegmentationTemplate(referenceImage);
  std::string name = this->CreateNodeName("scribble");

  m_ScribbleNode = LabelSetImageHelper::CreateNewSegmentationNode(nullptr, templateImage, name);
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

void mitk::nnInteractiveTool::OnScribbleEvent(itk::Object* caller, const itk::EventObject& event)
{
  if (this->IsNewTimePoint())
  {//set current 3D image vol into the nnInter session
    this->SetImageInSession();
  }
  this->UpdatePreview();
}

void mitk::nnInteractiveTool::RemoveScribbleNode()
{
  if (m_ScribbleNode.IsNull())
    return;

  this->GetDataStorage()->Remove(m_ScribbleNode);
  m_ScribbleNode = nullptr;

  m_ScribbleLabels.clear();
}

void mitk::nnInteractiveTool::AddLassoMaskNode(PromptType promptType)
{
  if (m_LassoMaskNode.IsNotNull())
    return;

  auto referenceNode = this->GetToolManager()->GetReferenceData(0);
  auto referenceImage = referenceNode->GetDataAs<Image>();
  auto templateImage = SegmentationHelper::GetStaticSegmentationTemplate(referenceImage);
  std::string name = this->CreateNodeName("lasso mask", promptType);

  m_LassoMaskNode = LabelSetImageHelper::CreateNewSegmentationNode(nullptr, templateImage, name);
  m_LassoMaskNode->SetBoolProperty("helper object", true);
  m_LassoMaskNode->SetVisibility(false);

  auto labelName = this->GetPromptTypeString(promptType);
  const auto& color = this->GetColor(promptType, Intensity::Vibrant);
  auto label = m_LassoMaskNode->GetDataAs<LabelSetImage>()->AddLabel(name, color, 0);

  this->GetDataStorage()->Add(m_LassoMaskNode, referenceNode);
}

void mitk::nnInteractiveTool::OnLassoEvent(itk::Object* caller, const itk::EventObject& event)
{
  auto node = DataNode::New();
  node->SetData(static_cast<const LassoEvent*>(&event)->GetContour());
  node->SetName(this->CreateNodeName("lasso", m_PromptType));
  node->SetColor(this->GetColor(m_PromptType, Intensity::Muted), nullptr, "contour.color");
  node->SetFloatProperty("contour.width", 3.0f);
  node->SetBoolProperty("helper object", true);

  this->GetLassoNodes(m_PromptType).push_back(node);
  this->GetDataStorage()->Add(node, this->GetToolManager()->GetReferenceData(0));

  if (this->IsNewTimePoint())
  {//set current 3D image vol into the nnInter session
    this->SetImageInSession();
  }
  this->UpdatePreview();
}

const std::vector<mitk::DataNode::Pointer>& mitk::nnInteractiveTool::GetLassoNodes(PromptType promptType) const
{
  if (promptType == PromptType::Positive)
    return m_PositiveLassoNodes;

  return m_NegativeLassoNodes;
}

std::vector<mitk::DataNode::Pointer>& mitk::nnInteractiveTool::GetLassoNodes(PromptType promptType)
{
  if (promptType == PromptType::Positive)
    return m_PositiveLassoNodes;

  return m_NegativeLassoNodes;
}

void mitk::nnInteractiveTool::RemoveLassoMaskNode()
{
  if (m_LassoMaskNode.IsNull())
    return;

  this->GetDataStorage()->Remove(m_LassoMaskNode);
  m_LassoMaskNode = nullptr;
}

void mitk::nnInteractiveTool::AddPointInteraction(const Point3D& point) // remove --ashis
{
  MITK_INFO << "AddPointInteraction()";

  const bool prompt = m_PromptType == PromptType::Positive;

  MITK_INFO << "  " << point << ", " << prompt;
}

void mitk::nnInteractiveTool::AddBoxInteraction(const PlanarRectangle* box)
{
  MITK_INFO << "AddBoxInteraction()";

  const bool prompt = m_PromptType == PromptType::Positive;

  const auto p0 = box->GetWorldControlPoint(0);
  const auto p2 = box->GetWorldControlPoint(2);

  itk::FixedArray<itk::FixedArray<ScalarType, 2>, 3> bbox;

  for (int i = 0; i < 3; ++i)
  {
    bbox[i][0] = std::min(p0[i], p2[i]);
    bbox[i][1] = std::max(p0[i], p2[i]);
  }

  MITK_INFO << "  " << bbox << ", " << prompt;
}

void mitk::nnInteractiveTool::AddScribbleInteraction(const Image* mask)
{
  MITK_INFO << "AddScribbleInteraction() [" << this->GetPromptTypeString(m_PromptType) << ']';

  const bool prompt = m_PromptType == PromptType::Positive;
  const auto size = itk::MakeSize<unsigned int>(mask->GetDimension(0), mask->GetDimension(1), mask->GetDimension(2));

  MITK_INFO << "  Image" << mask->GetPixelType().GetTypeAsString() << ", " << size << ", " << prompt;
}

void mitk::nnInteractiveTool::AddLassoInteraction(const Image* mask)
{
  MITK_INFO << "AddLassoInteraction()";

  const bool prompt = m_PromptType == PromptType::Positive;
  const auto size = itk::MakeSize<unsigned int>(mask->GetDimension(0), mask->GetDimension(1), mask->GetDimension(2));

  MITK_INFO << "  Image" << mask->GetPixelType().GetTypeAsString() << ", " << size << ", " << prompt;
}

void mitk::nnInteractiveTool::AddInitialSegInteraction(/*const*/ Image *mask)
{
  MITK_INFO << "AddInitialSegInteraction()";
  m_MaskImage = mask;
  m_ActiveTool.reset();
  if (this->IsNewTimePoint())
  { // set current 3D image vol into the nnInter session
    this->SetImageInSession();
  }
  this->UpdatePreview();
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

void mitk::nnInteractiveTool::CreatePointInteractor()
{
  m_PointInteractor = PointSetDataInteractor::New();
  m_PointInteractor->LoadStateMachine("PointSet.xml");
  m_PointInteractor->SetEventConfig("PointSetConfigLMB.xml");
  m_PointInteractor->EnableInteraction(false);
  m_PointInteractor->EnableMovement(false);
  m_PointInteractor->EnableRemoval(false);
}

void mitk::nnInteractiveTool::CreateBoxInteractor()
{
  auto module = us::ModuleRegistry::GetModule("MitkPlanarFigure");

  m_BoxInteractor = PlanarFigureInteractor::New();
  m_BoxInteractor->LoadStateMachine("PlanarFigureInteraction.xml", module);
  m_BoxInteractor->SetEventConfig("PlanarFigureConfig.xml", module);
  m_BoxInteractor->EnableInteraction(false);
}

void mitk::nnInteractiveTool::Activated()
{
  Superclass::Activated();

  m_PositivePointsNode = this->CreatePointSetNode(PromptType::Positive);
  m_NegativePointsNode = this->CreatePointSetNode(PromptType::Negative);

  this->CreatePointInteractor();
  this->CreateBoxInteractor();
}

void mitk::nnInteractiveTool::Deactivated()
{
  this->DisableInteraction();
  this->ResetInteractions();

  m_BoxInteractor = nullptr;
  m_PointInteractor = nullptr;

  this->GetDataStorage()->Remove(m_NegativePointsNode);
  m_NegativePointsNode = nullptr;

  this->GetDataStorage()->Remove(m_PositivePointsNode);
  m_PositivePointsNode = nullptr;

  this->CleanUpSession();
  this->IsSessionReadyOff();
  m_PythonContext = nullptr;
  m_LastSetTimePoint = std::numeric_limits<double>::lowest();
  Superclass::Deactivated();
}

std::pair<std::string, bool> mitk::nnInteractiveTool::GetPointAsListString(const mitk::BaseGeometry *baseGeometry) const
{
  const auto *pointSet = this->GetPointSetNode(m_PromptType)->GetDataAs<PointSet>();
  const auto point3d = pointSet->GetPoint(pointSet->GetSize() - 1);
  bool includeInteraction = m_PromptType == PromptType::Positive;
  std::stringstream pointsAndLabels;
  pointsAndLabels << "[";
  const char COMMA = ',';
  if (baseGeometry->IsInside(point3d))
  {
    mitk::Point3D index3D;
    baseGeometry->WorldToIndex(point3d, index3D);
    pointsAndLabels << static_cast<int>(index3D[2]) << COMMA << static_cast<int>(index3D[1]) << COMMA;
    pointsAndLabels << static_cast<int>(index3D[0]);
  }
  pointsAndLabels << "]";
  return std::make_pair(pointsAndLabels.str(), includeInteraction);
}

std::pair<std::string, bool> mitk::nnInteractiveTool::GetBBoxAsListString(const mitk::BaseGeometry* baseGeometry) const
{
  bool includeInteraction = m_PromptType == PromptType::Positive;
  const mitk::DataNode::Pointer node = this->GetBoxNodes(m_PromptType).back();
  const auto *box = node->GetDataAs<PlanarRectangle>();
  mitk::Point3D p0 = box->GetWorldControlPoint(0);
  mitk::Point3D p2 = box->GetWorldControlPoint(2);
  const char COMMA = ',';
  std::stringstream bboxPoints;
  bboxPoints << "[";
  if (baseGeometry->IsInside(p0) && baseGeometry->IsInside(p2))
  {
    mitk::Point3D index0, index2;
    baseGeometry->WorldToIndex(p0, index0);
    baseGeometry->WorldToIndex(p2, index2);
    for(int i = 2; i >= 0; --i)
    {
      bboxPoints << "[";
      bboxPoints << std::min(index0[i], index2[i]) << COMMA;
      bboxPoints << std::max(index0[i], index2[i]);
      bboxPoints << "],";
    }
    if (bboxPoints.tellp() > 1)
    {
      bboxPoints.seekp(-1, bboxPoints.end); // remove last added comma character
    }
  }
  bboxPoints << "]";
  return std::make_pair(bboxPoints.str(), includeInteraction);
}

void mitk::nnInteractiveTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                              const Image * /* oldSegAtTimeStep*/,
                                              LabelSetImage *previewImage,
                                              TimeStepType timeStep)
{
  if (nullptr != previewImage && m_PythonContext.IsNotNull())
  {
    m_ProgressCommand->SetProgress(20);
    this->SetSelectedLabels({MASK_VALUE});
    if (m_ActiveTool.has_value())
    {
      try
      {
        std::string pyCommand;
        bool includeInteraction = m_PromptType == PromptType::Positive;
        auto start = std::chrono::system_clock::now();
        switch (m_ActiveTool.value())
        {
          case Tool::Point:
          {
            std::pair<std::string, bool> pointInteraction = this->GetPointAsListString(inputAtTimeStep->GetGeometry());
            MITK_INFO << "Point: " << pointInteraction.first;
            pyCommand = "coordinate_list = " + pointInteraction.first + "\n"
                        "session.add_point_interaction(coordinate_list, include_interaction = ";
            includeInteraction = pointInteraction.second;
            break;
          }
          case Tool::Box:
          {
            std::pair<std::string, bool> bboxInteraction = GetBBoxAsListString(inputAtTimeStep->GetGeometry());
            MITK_INFO << "bboxInteraction: " << bboxInteraction.first;
            pyCommand = "bbox_list = " + bboxInteraction.first +"\n"
                        "session.add_bbox_interaction(bbox_list, include_interaction = ";
            includeInteraction = bboxInteraction.second;
            break;
          }
          case Tool::Scribble:
          {
            auto *scribbleMask = m_ScribbleNode->GetDataAs<mitk::Image>();
            m_PythonContext->TransferBaseDataToPython(scribbleMask, "_mitk_scribble_mask");
            pyCommand = "scribble_npy = _mitk_scribble_mask.GetAsNumpy()\n"
                        "session.add_scribble_interaction(scribble_npy.astype(numpy.uint8),"
                        "include_interaction = ";
            break;
          }
          case Tool::Lasso:
          {
            auto *lassoMask = m_LassoMaskNode->GetDataAs<mitk::Image>();
            m_PythonContext->TransferBaseDataToPython(lassoMask, "_mitk_lasso_mask");
            pyCommand = "lassoMask = _mitk_lasso_mask.GetAsNumpy()\n"
                        "session.add_scribble_interaction(lassoMask.astype(numpy.uint8),"
                        "include_interaction = ";
            break;
          }
          default:
            mitkThrow() << "Unsupported interaction type.";
            break;
        }
        m_ProgressCommand->SetProgress(50);

        if (includeInteraction)
        {
          pyCommand.append("True)\n");
        }
        else
        {
          pyCommand.append("False)\n");
        }

        pyCommand.append("session.set_do_prediction_propagation");
        if (m_AutoZoom)
        {
          pyCommand.append("(True)\n"); // Auto-zoom
        }
        else
        {
          pyCommand.append("(False)\n"); // Auto-zoom
        }

        m_PythonContext->ExecuteString(pyCommand);
        m_ProgressCommand->SetProgress(150);
        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        MITK_INFO << "nnInter tool inference elapsed: " << elapsed.count();

        m_ProgressCommand->SetProgress(180);
        mitk::ImageReadAccessor newMitkImgAcc(m_OutputBuffer.GetPointer());
        previewImage->SetVolume(newMitkImgAcc.GetData(), timeStep);
      }
      catch (const mitk::Exception &e)
      {
        mitkThrow() << e.GetDescription();
      }
    }
    else if(nullptr != m_MaskImage) // From mask
    {
      m_PythonContext->TransferBaseDataToPython(m_MaskImage, "_mitk_pre_mask");
      std::string pyCommand = "pre_mask_npy = _mitk_pre_mask.GetAsNumpy()\n"
                              "session.add_initial_seg_interaction(pre_mask_npy.astype(numpy.uint8))\n"
                              "session._predict()\n";
      m_PythonContext->ExecuteString(pyCommand);
      mitk::ImageReadAccessor newMitkImgAcc(m_OutputBuffer.GetPointer());
      previewImage->SetVolume(newMitkImgAcc.GetData(), timeStep);
    }
  }
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

void mitk::nnInteractiveTool::SetImageInSession()
{
  if (m_PythonContext.IsNull())
  {
    mitkThrow() << "Error: Python context was not activated.";
  }
  MITK_INFO << "in SetImageInSession......";
  auto *inputImage = dynamic_cast<Image *>(this->GetToolManager()->GetReferenceData(0)->GetData());
  const TimePointType timePoint =
    RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  mitk::Image::ConstPointer inputVolAtTimeStep = this->GetImageByTimePoint(inputImage, timePoint);
  mitk::Image *inputAtTimeStep = const_cast<mitk::Image *>(inputVolAtTimeStep.GetPointer());
  m_OutputBuffer->Initialize(inputAtTimeStep);
  m_PythonContext->TransferBaseDataToPython(inputAtTimeStep);
  m_PythonContext->TransferBaseDataToPython(m_OutputBuffer.GetPointer(), "_mitk_seg_image");
  mitk::Vector3D spacing = inputAtTimeStep->GetGeometry()->GetSpacing();
  std::string pycommand = "image_npy = _mitk_image.GetAsNumpy()\n"
                          "print('numpy image shape',image_npy.shape)\n"
                          "spacing = [" +
                          std::to_string(spacing[2]) + "," + std::to_string(spacing[1]) + "," + std::to_string(spacing[0]) +
                          "]\n"
                          "target_buffer_numpy = _mitk_seg_image.GetAsNumpy()\n"
                          "target_buffer = torch.from_numpy(target_buffer_numpy)\n"
                          "session.set_image(image_npy[None], {'spacing' : spacing})\n"
                          "session.set_target_buffer(target_buffer)\n";
  try
  {
    m_PythonContext->ExecuteString(pycommand);
  }
  catch (mitk::Exception &e)
  {
    mitkThrow() << e.GetDescription();
  }
  m_LastSetTimePoint = timePoint;
}

void mitk::nnInteractiveTool::InitializeBackend()
{
  this->IsSessionReadyOff();
  m_OutputBuffer = mitk::LabelSetImage::New();
  std::string modelName = "nnInteractive_v1.0";
  auto start = std::chrono::system_clock::now();
  m_PythonContext = mitk::PythonContext::New();
  m_PythonContext->Activate();
  m_PythonContext->SetVirtualEnvironmentPath("C:/DKFZ/nnInteractive_work/.venv/Lib/site-packages");
  std::string pycommand = "import torch\n"
                          "import nnInteractive\n"
                          "from pathlib import Path\n"
                          "from nnunetv2.utilities.find_class_by_name import recursive_find_python_class\n"
                          "from batchgenerators.utilities.file_and_folder_operations import join, load_json\n"
                          "from huggingface_hub import login, snapshot_download\n"
                          "login(token = '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')\n" // Private token
                          "repo_id = 'kraemer/nnInteractive'\n"
                          "download_path = snapshot_download(repo_id = repo_id, allow_patterns = ['" +
                          modelName + "/*'], force_download = False)\n"
                          //"download_path = os.getcwd()\n"
                          "checkpoint_path = Path(download_path).joinpath('" + modelName + "')\n" //hardcode model checkpoint path to pack together
                          "print(f'Using Model " + modelName + " at:{checkpoint_path}')\n";
  try
  {
    m_PythonContext->ExecuteString(pycommand);
  }
  catch (mitk::Exception &e)
  {
    mitkThrow() << e.GetDescription();
  }
  pycommand.clear();
  pycommand = "if Path(checkpoint_path).joinpath('inference_session_class.json').is_file():\n"
              "   inference_class = load_json(Path(checkpoint_path).joinpath('inference_session_class.json'))\n"
              "if isinstance (inference_class, dict):\n"
              "   inference_class = inference_class['inference_class']\n"
              "else:\n"
              "   inference_class = 'nnInteractiveInferenceSessionV3'\n"
              "inference_class = recursive_find_python_class(join(nnInteractive.__path__[0], 'inference'),"
              "inference_class, 'nnInteractive.inference')\n";
  try
  {
    m_PythonContext->ExecuteString(pycommand);
  }
  catch (mitk::Exception &e)
  {
    mitkThrow() << e.GetDescription();
  }
  pycommand.clear();
  pycommand = "session = inference_class("
              "device = torch.device('cuda:0'),"
              "use_torch_compile = False,"
              "torch_n_threads = 16,"
              "verbose = True,"
              "use_background_preprocessing = False,"
              "do_prediction_propagation = True)\n"
              "session.initialize_from_trained_model_folder(checkpoint_path, 0, 'checkpoint_final.pth')\n ";
  try
  {
    m_PythonContext->ExecuteString(pycommand);
  }
  catch (mitk::Exception& e)
  {
    mitkThrow() << e.GetDescription();
  }

  this->SetImageInSession();
 
  auto end = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  MITK_INFO << "nnInteractive init elapsed: " << elapsed.count();
  this->IsSessionReadyOn();
}

void mitk::nnInteractiveTool::ConfirmCleanUp()
{
  nnInterConfirmMessageEvent.Send(true);
  RenderingManager::GetInstance()->RequestUpdateAll();
}
