/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveScribbleInteractor.h>

#include <mitkDrawPaintbrushTool.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkSegmentationHelper.h>
#include <mitkToolManager.h>

namespace
{
  // Internal event triggered when a brushstroke is completed.
  class ScribbleEvent : public itk::AnyEvent
  {
  public:
    using Self = ScribbleEvent;
    using Superclass = itk::AnyEvent;

    explicit ScribbleEvent(mitk::Image* mask = nullptr)
      : m_Mask(mask)
    {
    }

    ScribbleEvent(const Self& other)
      : Superclass(other),
        m_Mask(other.m_Mask)
    {
    }

    ~ScribbleEvent() override
    {
    }

    const char* GetEventName() const override
    {
      return "ScribbleEvent";
    }

    bool CheckEvent(const itk::EventObject* event) const override
    {
      return dynamic_cast<const Self*>(event) != nullptr;
    }

    itk::EventObject* MakeObject() const override
    {
      return new Self(*this);
    }

    mitk::Image* GetMask() const
    {
      return m_Mask;
    }

  private:
    mitk::Image::Pointer m_Mask;
  };

  // Allocate and initialize a 3D image volume with zeros, then transfer
  // ownership of the memory to the given image.
  void InitializeVolume(mitk::Image* image)
  {
    size_t numPixels = 1;

    for (int i = 0; i < 3; ++i)
      numPixels *= image->GetDimension(i);

    auto data = new mitk::Label::PixelType[numPixels];
    std::memset(data, 0, numPixels * sizeof(mitk::Label::PixelType));

    image->SetImportVolume(data, 0, 0, mitk::Image::ManageMemory);
  }

  // A wrapper for the DrawPaintbrushTool that mainly ensures compatibility with
  // the ToolManager and 3D interpolation feature of the Segmentation view
  // without interference.
  class ScribbleTool : public mitk::DrawPaintbrushTool
  {
  public:
    mitkClassMacro(ScribbleTool, DrawPaintbrushTool)
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
      return "Scribble";
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

      m_PaintingNode->SetColor(color);
      m_PaintingNode->SetBoolProperty("outline binary", false);
      this->SetFeedbackContourColor(color);
    }

    void Deactivate()
    {
      this->Disable();
      this->Deactivated();
      this->SetEnable3DInterpolation(true);
    }

  protected:
    ScribbleTool()
    {
      this->DisableContourMarkers();
      this->SetSize(3);
      this->Disable();

      // nnInteractive currently does not support undo.
      this->EnableUndo(false);
    }

    ~ScribbleTool() override
    {
    }

    int GetFillValue() const override
    {
      return 1;
    }

    void OnMouseReleased(mitk::StateMachineAction* action, mitk::InteractionEvent* event) override
    {
      // Although the interaction occurs on a single slice, we must generate
      // a corresponding 3D image mask containing the brushstroke, as
      // nnInteractive requires a full 3D mask as input.
      auto mask = mitk::Image::New();

      // This initializes only the metadata of the image.
      mask->Initialize(this->GetWorkingData()->GetGroupImage(0)); //we can just use the first group
                                                                  //as content is not relevant but
                                                                  //just the image geometry

      // Allocate and initialize the image volume based on the metadata.
      InitializeVolume(mask);

      SegTool2D::WriteSliceToVolume(mask, m_CurrentPlane, m_PaintingSlice, 0);

      Superclass::OnMouseReleased(action, event);

      // Notify ScribbleInteractor.
      this->InvokeEvent(ScribbleEvent(mask));
    }

    void OnInvertLogic(mitk::StateMachineAction*, mitk::InteractionEvent*) override
    {
      // Inversion is disabled; handled by ScribbleInteractor instead.
    }
  };
}

namespace mitk::nnInteractive
{
  class ScribbleInteractor::Impl
  {
  public:
    explicit Impl(ScribbleInteractor* owner)
      : Tool(ScribbleTool::New()),
        m_Owner(owner)
    {
      // Get notified each time a brushstroke is drawn.
      auto command = itk::MemberCommand<Impl>::New();
      command->SetCallbackFunction(this, &Impl::OnScribbleEvent);
      this->Tool->AddObserver(ScribbleEvent(), command);
    }

    void CreateScribbleNode()
    {
      if (this->ScribbleNode.IsNotNull())
        return; // Nothing to do.

      // Create a static scribble node based on the reference image.
      auto referenceNode = m_Owner->GetToolManager()->GetReferenceData(0);
      auto referenceImage = referenceNode->GetDataAs<Image>();
      auto templateImage = SegmentationHelper::GetStaticSegmentationTemplate(referenceImage);

      this->ScribbleNode = LabelSetImageHelper::CreateNewSegmentationNode(nullptr, templateImage, "Scribble");
      this->ScribbleNode->SetBoolProperty("helper object", true);

      // Add labels for all prompt types.
      for (auto promptType : GetAllPromptTypes())
        this->CreateScribbleLabel(promptType);

      m_Owner->GetDataStorage()->Add(this->ScribbleNode, referenceNode);
    }

    void DestroyScribbleNode()
    {
      if (this->ScribbleNode.IsNull())
        return; // Nothing to do.

      // Remove the scribble node from the data storage and release our reference to it.
      if (auto dataStorage = m_Owner->GetDataStorage(); dataStorage != nullptr)
        dataStorage->Remove(this->ScribbleNode);

      this->ScribbleNode = nullptr;

      // Clear the prompt type to label/pixel value map.
      m_ScribbleLabels.clear();

      // Release the mask of the last scribble.
      this->LastScribbleMask = nullptr;
    }

    void SetActiveScribbleLabel(PromptType promptType)
    {
      this->ScribbleNode->GetDataAs<MultiLabelSegmentation>()->SetActiveLabel(m_ScribbleLabels[promptType]);
      this->ScribbleNode->SetBoolProperty("labelset.contour.active", false);
    }

    bool IsScribbleEmpty() const
    {
      if (this->ScribbleNode.IsNull())
        return true;

      // Check if all labels of the scribble node are empty.
      for (const auto [promptType, pixelValue] : m_ScribbleLabels)
      {
        if (!this->ScribbleNode->GetDataAs<MultiLabelSegmentation>()->IsEmpty(pixelValue))
          return false;
      }

      return true;
    }

    ScribbleTool::Pointer Tool;
    DataNode::Pointer ScribbleNode;
    Image::Pointer LastScribbleMask;

  private:
    void CreateScribbleLabel(PromptType promptType)
    {
      const auto& name = GetPromptTypeAsString(promptType);
      const auto& color = GetColor(promptType, ColorIntensity::Muted);

      // Create an unlocked label for the given prompt type.
      auto label = this->ScribbleNode->GetDataAs<MultiLabelSegmentation>()->AddLabel(name, color, 0);
      label->SetLocked(false);

      // Store a mapping from the prompt type to the label/pixel value.
      m_ScribbleLabels[promptType] = label->GetValue();
    }

    void OnScribbleEvent(itk::Object*, const itk::EventObject& event)
    {
      this->LastScribbleMask = static_cast<const ScribbleEvent*>(&event)->GetMask();

      // Notify the nnInteractiveTool that an interaction has occurred.
      m_Owner->UpdatePreviewEvent(false);
    }

    ScribbleInteractor* m_Owner;
    std::unordered_map<PromptType, Label::PixelType> m_ScribbleLabels;
  };
}

mitk::nnInteractive::ScribbleInteractor::ScribbleInteractor()
  : Interactor(InteractionType::Scribble),
    m_Impl(std::make_unique<Impl>(this))
{
}

mitk::nnInteractive::ScribbleInteractor::~ScribbleInteractor()
{
  this->OnReset();
}

bool mitk::nnInteractive::ScribbleInteractor::HasInteractions() const
{
  // If any of the labels of the scribble node is not empty, interactions have occurred.
  return !m_Impl->IsScribbleEmpty();
}

const mitk::Image* mitk::nnInteractive::ScribbleInteractor::GetLastScribbleMask() const
{
  return m_Impl->LastScribbleMask;
}

void mitk::nnInteractive::ScribbleInteractor::OnSetToolManager()
{
  m_Impl->Tool->InitializeStateMachine();
  m_Impl->Tool->SetToolManager(this->GetToolManager());
}

void mitk::nnInteractive::ScribbleInteractor::OnHandleEvent(InteractionEvent* event)
{
  m_Impl->Tool->HandleEvent(event, nullptr);
}

void mitk::nnInteractive::ScribbleInteractor::OnEnable()
{
  auto promptType = this->GetCurrentPromptType();

  // Ensure the scribble node exists and set the active label based on the
  // current prompt type.
  m_Impl->CreateScribbleNode();
  m_Impl->SetActiveScribbleLabel(promptType);

  // Assign the scribble node as the working data for the ToolManager.
  this->GetToolManager()->SetWorkingData(m_Impl->ScribbleNode);

  // Activate the ScribbleTool with a color corresponding to the prompt type.
  const auto& color = GetColor(promptType, ColorIntensity::Vibrant);
  m_Impl->Tool->Activate(color);
}

void mitk::nnInteractive::ScribbleInteractor::OnDisable()
{
  // Deactivate the ScribbleTool.
  m_Impl->Tool->Deactivate();

  // Clear the ToolManager's working data.
  this->GetToolManager()->SetWorkingData(nullptr);

  // Remove the scribble node if it contains no data.
  if (m_Impl->IsScribbleEmpty())
    m_Impl->DestroyScribbleNode();
}

void mitk::nnInteractive::ScribbleInteractor::OnReset()
{
  m_Impl->DestroyScribbleNode();
}
