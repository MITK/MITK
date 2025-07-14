/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveLassoInteractor.h>

#include <mitkAddContourTool.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkSegmentationHelper.h>
#include <mitkToolManager.h>

namespace
{
  // Internal event triggered when a contour is completed.
  class LassoEvent : public itk::AnyEvent
  {
  public:
    using Self = LassoEvent;
    using Superclass = itk::AnyEvent;

    explicit LassoEvent(mitk::ContourModel* contour = nullptr)
      : m_Contour(contour)
    {
    }

    LassoEvent(const Self& other)
      : Superclass(other),
        m_Contour(other.m_Contour)
    {
    }

    ~LassoEvent() override
    {
    }

    const char* GetEventName() const override
    {
      return "LassoEvent";
    }

    bool CheckEvent(const itk::EventObject* event) const override
    {
      return dynamic_cast<const Self*>(event) != nullptr;
    }

    itk::EventObject* MakeObject() const override
    {
      return new Self(*this);
    }

    mitk::ContourModel* GetContour() const
    {
      return m_Contour;
    }

  private:
    mitk::ContourModel::Pointer m_Contour;
  };

  // A wrapper for the AddContourTool that mainly ensures compatibility with
  // the ToolManager and 3D interpolation feature of the Segmentation view
  // without interference.
  class LassoTool : public mitk::AddContourTool
  {
  public:
    mitkClassMacro(LassoTool, AddContourTool)
    itkFactorylessNewMacro(Self)

    using Superclass::SetToolManager;

    us::ModuleResource GetCursorIconResource() const override
    {
      return us::ModuleResource();
    }

    us::ModuleResource GetIconResource() const override
    {
      return us::ModuleResource();
    }

    const char* GetName() const override
    {
      return "Lasso";
    }

    bool IsEligibleForAutoInit() const override
    {
      return false;
    }

    void Activate(const mitk::Color& color)
    {
      this->SetEnable3DInterpolation(false);
      this->Activated();
      this->Enable();

      this->SetFeedbackContourColor(color);
      this->SetFeedbackContourWidth(3.0f);
    }

    void Deactivate()
    {
      this->Disable();
      this->Deactivated();
      this->SetEnable3DInterpolation(true);
    }

  protected:
    LassoTool()
    {
      this->DisableContourMarkers();
      this->Disable();

      // nnInteractive currently does not support undo.
      this->EnableUndo(false);
    }

    ~LassoTool() override
    {
    }

    void OnMouseReleased(mitk::StateMachineAction* action, mitk::InteractionEvent* event) override
    {
      // Do not process accidental user clicks.
      if (this->GetFeedbackContour()->GetNumberOfVertices() < 3)
        return;

      // Clone the feedback contour to preserve it, as it would otherwise be overriden.
      auto contour = this->GetFeedbackContour()->Clone();

      Superclass::OnMouseReleased(action, event);

      // Notify LassoInteractor.
      this->InvokeEvent(LassoEvent(contour));

      // Clear the working mask to prepare for the next interaction.
      this->GetWorkingData()->ClearGroupImage(0);
    }

    void OnInvertLogic(mitk::StateMachineAction*, mitk::InteractionEvent*) override
    {
      // Inversion is disabled; handled by LassoInteractor instead.
    }
  };
}

namespace mitk::nnInteractive
{
  class LassoInteractor::Impl
  {
  public:
    explicit Impl(LassoInteractor* owner)
      : Tool(LassoTool::New()),
        m_Owner(owner)
    {
      // Get notified each time a countour is drawn.
      auto command = itk::MemberCommand<Impl>::New();
      command->SetCallbackFunction(this, &Impl::OnLassoEvent);
      this->Tool->AddObserver(LassoEvent(), command);
    }

    ~Impl()
    {
    }

    void CreateMaskNode()
    {
      if (this->MaskNode.IsNotNull())
        return; // Nothing to do.

      // Create a static mask node based on the reference image.
      auto referenceNode = m_Owner->GetToolManager()->GetReferenceData(0);
      auto referenceImage = referenceNode->GetDataAs<Image>();
      auto templateImage = SegmentationHelper::GetStaticSegmentationTemplate(referenceImage);

      this->MaskNode = LabelSetImageHelper::CreateNewSegmentationNode(nullptr, templateImage, "Lasso mask");
      this->MaskNode->SetBoolProperty("helper object", true);
      this->MaskNode->SetVisibility(false);

      // Add a single label based on the current prompt type.
      const auto promptType = m_Owner->GetCurrentPromptType();
      const auto& labelName = GetPromptTypeAsString(promptType);
      const auto& color = GetColor(promptType, ColorIntensity::Vibrant);

      auto mask = this->MaskNode->GetDataAs<MultiLabelSegmentation>();
      auto label = mask->AddLabel(labelName, color, 0);
      mask->SetActiveLabel(label->GetValue());

      m_Owner->GetDataStorage()->Add(this->MaskNode, referenceNode);
    }

    void DestroyMaskNode()
    {
      if (this->MaskNode.IsNull())
        return; // Nothing to do.

      // Remove the mask node from the data storage and release our reference to it.
      m_Owner->GetDataStorage()->Remove(this->MaskNode);
      this->MaskNode = nullptr;
    }

    void DestroyLassoNodes()
    {
      // Remove all lasso nodes from the data storage and release their
      // references for each prompt type. This should result in their
      // destruction.
      for (auto& [promptType, nodes] : m_LassoNodes)
      {
        if (auto dataStorage = m_Owner->GetDataStorage(); dataStorage != nullptr)
        {
          for (const auto& node : nodes)
            dataStorage->Remove(node);
        }

        nodes.clear();
      }

      m_LassoNodes.clear();
    }

    bool HasInteractions() const
    {
      // If any list of lassos for a prompt type is not empty, interactions have occurred.
      for (const auto& [promptType, lassoNodes] : m_LassoNodes)
      {
        if (!lassoNodes.empty())
          return true;
      }

      return false;
    }

    LassoTool::Pointer Tool;
    DataNode::Pointer MaskNode;

  private:
    void OnLassoEvent(itk::Object*, const itk::EventObject& event)
    {
      const auto promptType = m_Owner->GetCurrentPromptType();

      // Create a node for the drawn contour, add it to the corresponding list
      // of lasso nodes based on prompt type, and add it to the data storage.
      auto node = DataNode::New();
      node->SetData(static_cast<const LassoEvent*>(&event)->GetContour());
      node->SetName(this->CreateLassoNodeName());
      node->SetColor(GetColor(promptType, ColorIntensity::Muted), nullptr, "contour.color");
      node->SetFloatProperty("contour.width", 3.0f);
      node->SetBoolProperty("helper object", true);

      m_LassoNodes[promptType].push_back(node);
      m_Owner->GetDataStorage()->Add(node, m_Owner->GetToolManager()->GetReferenceData(0));

      // Notify the nnInteractiveTool that an interaction has occurred.
      m_Owner->UpdatePreviewEvent(false);
    }

    std::string CreateLassoNodeName()
    {
      const auto& promptType = GetPromptTypeAsString(m_Owner->GetCurrentPromptType());
      return m_Owner->GetDataStorage()->GetUniqueName(promptType + " lasso");
    }

    LassoInteractor* m_Owner;
    std::unordered_map<PromptType, std::vector<DataNode::Pointer>> m_LassoNodes;
  };
}

mitk::nnInteractive::LassoInteractor::LassoInteractor()
  : Interactor(InteractionType::Lasso),
    m_Impl(std::make_unique<Impl>(this))
{
}

mitk::nnInteractive::LassoInteractor::~LassoInteractor()
{
  this->OnReset();
}

bool mitk::nnInteractive::LassoInteractor::HasInteractions() const
{
  return m_Impl->HasInteractions();
}

const mitk::Image* mitk::nnInteractive::LassoInteractor::GetLastLassoMask() const
{
  return m_Impl->MaskNode->GetDataAs<MultiLabelSegmentation>()->GetGroupImage(0);
}

void mitk::nnInteractive::LassoInteractor::OnSetToolManager()
{
  m_Impl->Tool->InitializeStateMachine();
  m_Impl->Tool->SetToolManager(this->GetToolManager());
}

void mitk::nnInteractive::LassoInteractor::OnHandleEvent(InteractionEvent* event)
{
  m_Impl->Tool->HandleEvent(event, nullptr);
}

void mitk::nnInteractive::LassoInteractor::OnEnable()
{
  // Create a mask node for the most recently drawn contour, which is used both
  // as target by the LassoTool and as input for nnInteractive.
  m_Impl->CreateMaskNode();
  this->GetToolManager()->SetWorkingData(m_Impl->MaskNode);

  // Activate the LassoTool with a color corresponding to the prompt type.
  const auto& color = GetColor(this->GetCurrentPromptType(), ColorIntensity::Vibrant);
  m_Impl->Tool->Activate(color);
}

void mitk::nnInteractive::LassoInteractor::OnDisable()
{
  // Deactivate the LassoTool.
  m_Impl->Tool->Deactivate();

  // Reset the working data of the ToolManager and destroy the mask node for
  // the most recently drawn contour.
  this->GetToolManager()->SetWorkingData(nullptr);
  m_Impl->DestroyMaskNode();
}

void mitk::nnInteractive::LassoInteractor::OnReset()
{
  m_Impl->DestroyMaskNode();
  m_Impl->DestroyLassoNodes();
}
