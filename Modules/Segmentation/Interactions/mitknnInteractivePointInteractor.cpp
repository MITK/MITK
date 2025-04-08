/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractivePointInteractor.h>

#include <mitkPointSetDataInteractor.h>
#include <mitkPointSetShapeProperty.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkToolManager.h>

namespace mitk::nnInteractive
{
  class PointInteractor::Impl
  {
  public:
    explicit Impl(PointInteractor* owner)
      : Interactor(PointSetDataInteractor::New()),
        m_Owner(owner)
    {
      this->Interactor->LoadStateMachine("PointSet.xml");

      // We use a customized event configuration that allows placing points
      // without the need for any modifier key, at the cost of blocking
      // crosshair navigation via left clicks.
      this->Interactor->SetEventConfig("PointSetConfigLMB.xml");

      // nnInteractive currently does not support undo.
      this->Interactor->EnableUndo(false);
      this->Interactor->EnableInteraction(false);
      this->Interactor->EnableMovement(false);
      this->Interactor->EnableRemoval(false);
    }

    ~Impl()
    {
    }

    void CreatePointSetNode()
    {
      const auto promptType = m_Owner->GetCurrentPromptType();

      // Check if a point set node for the current prompt type has already been created.
      if (this->PointSetNodes.find(promptType) != this->PointSetNodes.end())
        return;

      auto pointSet = PointSet::New();

      // Ensure that the new point set is visible at the current time point.
      auto geometry = static_cast<ProportionalTimeGeometry*>(pointSet->GetTimeGeometry());
      geometry->SetStepDuration(std::numeric_limits<TimePointType>::max());

      // Whenever a point is added to the point set, we need to inform
      // nnInteractive about the interaction.
      auto command = itk::SimpleMemberCommand<Impl>::New();
      command->SetCallbackFunction(this, &Impl::OnPointSetAddEvent);
      pointSet->AddObserver(PointSetAddEvent(), command);

      // Style the point set using data node properties, taking into account
      // the current prompt type.
      auto node = DataNode::New();
      node->SetData(pointSet);
      node->SetName(GetPromptTypeAsString(promptType) + " points");
      node->SetColor(GetColor(promptType, ColorIntensity::Muted));
      node->SetColor(GetColor(promptType, ColorIntensity::Vibrant), nullptr, "selectedcolor");
      node->SetProperty("Pointset.2D.shape", PointSetShapeProperty::New(PointSetShapeProperty::CIRCLE));
      node->SetIntProperty("Pointset.2D.resolution", 64);
      node->SetFloatProperty("point 2D size", 5.0f);
      node->SetFloatProperty("Pointset.2D.distance to plane", 0.1f);
      node->SetBoolProperty("Pointset.2D.keep shape when selected", true);
      node->SetBoolProperty("Pointset.2D.fill shape", true);
      node->SetBoolProperty("helper object", true);

      // The creation is complete. Now, it's time to commit our progress by
      // storing the point set data node for the current prompt type as a
      // persistent class member and adding it to the data storage.
      this->PointSetNodes[promptType] = node;
      m_Owner->GetDataStorage()->Add(node, m_Owner->GetToolManager()->GetReferenceData(0));
    }

    void DestroyPointSetNode(PromptType promptType)
    {
      // Check if a point set exists for the given prompt type.
      if (auto iter = this->PointSetNodes.find(promptType); iter != this->PointSetNodes.end())
      {
        // Remove the point set from the data storage and erase our reference to it.
        m_Owner->GetDataStorage()->Remove(iter->second);
        this->PointSetNodes.erase(iter);
      }
    }

    void OnPointSetAddEvent()
    {
      // Notify the nnInteractiveTool that an interaction has occurred.
      m_Owner->UpdatePreviewEvent.Send(false);
    }

    PointSetDataInteractor::Pointer Interactor;
    std::unordered_map<PromptType, DataNode::Pointer> PointSetNodes;

  private:
    PointInteractor* m_Owner;
  };
}

mitk::nnInteractive::PointInteractor::PointInteractor()
  : Interactor(InteractionType::Point, InteractionMode::BlockLMBDisplayInteraction),
    m_Impl(std::make_unique<Impl>(this))
{
}

mitk::nnInteractive::PointInteractor::~PointInteractor()
{
  this->OnReset();
}

bool mitk::nnInteractive::PointInteractor::HasInteractions() const
{
  // If any point set for a prompt type is not empty, interactions have occurred.
  for (const auto& [promptType, pointSetNode] : m_Impl->PointSetNodes)
  {
    if (pointSetNode->GetDataAs<PointSet>()->GetSize() != 0)
      return true;
  }

  return false;
}

std::optional<mitk::Point3D> mitk::nnInteractive::PointInteractor::GetLastPoint() const
{
  const auto promptType = this->GetCurrentPromptType();

  if (m_Impl->PointSetNodes.find(promptType) == m_Impl->PointSetNodes.end())
    return std::nullopt;

  auto pointSet = m_Impl->PointSetNodes[promptType]->GetDataAs<PointSet>();

  if (pointSet->IsEmpty())
    return std::nullopt;

  return pointSet->GetPoint(pointSet->GetSize() - 1);
}

void mitk::nnInteractive::PointInteractor::OnEnable()
{
  // Create a point set for the current prompt type if it doesn't already exist.
  m_Impl->CreatePointSetNode();

  // Enable the actual interactor for the appropriate point set and restrict
  // points to the bounds of the reference image.
  auto referenceGeometry = this->GetToolManager()->GetReferenceData(0)->GetData()->GetGeometry();

  m_Impl->Interactor->SetBounds(referenceGeometry);
  m_Impl->Interactor->SetDataNode(m_Impl->PointSetNodes.at(this->GetCurrentPromptType()));
  m_Impl->Interactor->EnableInteraction(true);
}

void mitk::nnInteractive::PointInteractor::OnDisable()
{
  // Disable interaction and detach the actual interactor.
  m_Impl->Interactor->EnableInteraction(false);
  m_Impl->Interactor->SetDataNode(nullptr);
  m_Impl->Interactor->SetBounds(nullptr);

  const auto promptType = this->GetCurrentPromptType();
  auto pointSet = m_Impl->PointSetNodes.at(promptType)->GetDataAs<PointSet>();

  if (pointSet->IsEmpty())
  {
    // If the point set is empty, destroy it.
    m_Impl->DestroyPointSetNode(promptType);
  }
  else
  {
    // If the point set contains points, clear its selection to remove visual highlighting.
    pointSet->ClearSelection();
  }
}

void mitk::nnInteractive::PointInteractor::OnReset()
{
  // Destroy all point sets.
  for (auto promptType : GetAllPromptTypes())
    m_Impl->DestroyPointSetNode(promptType);
}
