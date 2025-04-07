/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveBoxInteractor.h>

#include <mitkPlanarFigureInteractor.h>
#include <mitkPlanarRectangle.h>
#include <mitkToolManager.h>

#include <usModuleRegistry.h>

#include <optional>

namespace mitk::nnInteractive
{
  class BoxInteractor::Impl
  {
  public:
    explicit Impl(BoxInteractor* owner)
      : m_Owner(owner),
        m_Interactor(PlanarFigureInteractor::New())
    {
      auto planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");

      m_Interactor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
      m_Interactor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
      m_Interactor->EnableInteraction(false);

      // nnInteractive currently does not support undo.
      m_Interactor->EnableUndo(false);
    }

    ~Impl()
    {
    }

    void CreateNextBoxNode()
    {
      this->ReleaseNextBoxNode();

      auto box = PlanarRectangle::New();

      // Whenever a box is completed, we need to add it to the list of drawn
      // boxes, inform nnInteractive about the interaction and create a new
      // next box node.
      auto command = itk::SimpleMemberCommand<Impl>::New();
      command->SetCallbackFunction(this, &Impl::OnEndPlacementPlanarFigureEvent);
      const auto tag = box->AddObserver(EndPlacementPlanarFigureEvent(), command);

      const auto promptType = m_Owner->GetCurrentPromptType();

      // Style the box using data node properties, taking into account the
      // current prompt type.
      auto node = DataNode::New();
      node->SetName(this->CreateNextBoxNodeName());
      node->SetData(box);
      node->SetColor(GetColor(promptType, ColorIntensity::Muted));
      node->SetColor(GetColor(promptType, ColorIntensity::Vibrant), nullptr, "planarfigure.selected.line.color");
      node->SetSelected(true);
      node->SetFloatProperty("planarfigure.line.width", 3.0f);
      node->SetBoolProperty("planarfigure.drawname", false);
      node->SetBoolProperty("planarfigure.drawshadow", false);
      node->SetBoolProperty("planarfigure.drawcontrolpoints", false);
      node->SetBoolProperty("planarfigure.hidecontrolpointsduringinteraction", true);
      node->SetBoolProperty("planarfigure.fill", true);
      node->SetBoolProperty("helper object", true);

      // The creation is complete. Now, it's time to commit our progress by
      // storing the next box data node as a persistent class member and
      // adding it to the data storage.
      m_NextBoxNode = node;
      m_EndPlacementObserverTag = tag;

      m_Owner->GetDataStorage()->Add(node, m_Owner->GetToolManager()->GetReferenceData(0));

      // Finally, enable interaction for the next box.
      m_Interactor->SetDataNode(m_NextBoxNode);
      m_Interactor->EnableInteraction(true);
    }

    void ReleaseNextBoxNode()
    {
      // NOTE: This method is called Release instead of Destroy as the node is
      // usually not destroyed but still referenced by the list of box nodes.
      // However, it may be destroyed in other cases.

      if (m_NextBoxNode.IsNull())
        return; // Nothing to do.

      // Disable interaction and detach the actual interactor.
      m_Interactor->EnableInteraction(false);
      m_Interactor->SetDataNode(nullptr);

      // Remove the box from data storage if it was never completed, as it is
      // not yet managed by the list of boxes.
      if (m_NextBoxNode->GetData()->GetProperty("initiallyplaced").IsNull())
        m_Owner->GetDataStorage()->Remove(m_NextBoxNode);

      // Unsubscribe from the EndPlacementPlanarFigureEvent.
      if (m_EndPlacementObserverTag.has_value())
      {
        m_NextBoxNode->RemoveObserver(m_EndPlacementObserverTag.value());
        m_EndPlacementObserverTag.reset();
      }

      // Release the reference to the next box.
      m_NextBoxNode = nullptr;
    }

    void DestroyBoxNodes()
    {
      // Remove all box nodes from the data storage and release their
      // references for each prompt type. This should result in their
      // destruction.
      for (auto& [promptType, nodes] : this->BoxNodes)
      {
        for (const auto& node : nodes)
          m_Owner->GetDataStorage()->Remove(node);

        nodes.clear();
      }

      this->BoxNodes.clear();
    }

    std::unordered_map<PromptType, std::vector<DataNode::Pointer>> BoxNodes;

  private:
    void OnEndPlacementPlanarFigureEvent()
    {
      // Add the newly placed box to the list of completed boxes.
      auto latestBoxNode = m_NextBoxNode;
      latestBoxNode->SetName(this->CreateBoxNodeName());
      latestBoxNode->SetSelected(false);
      this->BoxNodes[m_Owner->GetCurrentPromptType()].push_back(latestBoxNode);

      // Notify the nnInteractiveTool that an interaction has occurred.
      m_Owner->UpdatePreviewEvent(false);

      // Create a new box for continous interaction.
      this->CreateNextBoxNode();
    }

    std::string CreateNextBoxNodeName()
    {
      auto promptType = GetPromptTypeAsString(m_Owner->GetCurrentPromptType());

      // Convert prompt type to lowercase.
      for (auto& c : promptType)
        c = std::tolower(c);

      return "Next " + promptType + " box";
    }

    std::string CreateBoxNodeName()
    {
      const auto& promptType = GetPromptTypeAsString(m_Owner->GetCurrentPromptType());
      return m_Owner->GetDataStorage()->GetUniqueName(promptType + " box");
    }

    BoxInteractor* m_Owner;
    PlanarFigureInteractor::Pointer m_Interactor;
    DataNode::Pointer m_NextBoxNode;
    std::optional<unsigned long> m_EndPlacementObserverTag;
  };
}

mitk::nnInteractive::BoxInteractor::BoxInteractor()
  : Interactor(InteractionType::Box, InteractionMode::BlockLMBDisplayInteraction),
    m_Impl(std::make_unique<Impl>(this))
{
}

mitk::nnInteractive::BoxInteractor::~BoxInteractor()
{
  this->OnReset();
}

bool mitk::nnInteractive::BoxInteractor::HasInteractions() const
{
  // If any list of boxes for a prompt type is not empty, interactions have occurred.
  for (const auto& [promptType, boxNodes] : m_Impl->BoxNodes)
  {
    if (!boxNodes.empty())
      return true;
  }

  return false;
}

const mitk::PlanarFigure* mitk::nnInteractive::BoxInteractor::GetLastBox() const
{
  const auto promptType = this->GetCurrentPromptType();

  if (m_Impl->BoxNodes.find(promptType) == m_Impl->BoxNodes.end())
    return nullptr;

  if (m_Impl->BoxNodes[promptType].empty())
    return nullptr;

  return m_Impl->BoxNodes[promptType].back()->GetDataAs<PlanarFigure>();
}

void mitk::nnInteractive::BoxInteractor::OnEnable()
{
  m_Impl->CreateNextBoxNode();
}

void mitk::nnInteractive::BoxInteractor::OnDisable()
{
  m_Impl->ReleaseNextBoxNode();
}

void mitk::nnInteractive::BoxInteractor::OnReset()
{
  m_Impl->ReleaseNextBoxNode();
  m_Impl->DestroyBoxNodes();
}
