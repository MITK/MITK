/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveLassoInteractor.h>

#include <mitkBaseRenderer.h>
#include <mitkContourModel.h>
#include <mitkContourModelUtils.h>
#include <mitkEventStateMachine.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkSegTool2D.h>
#include <mitkToolManager.h>

#include <usModuleRegistry.h>

#include "mitknnInteractiveBoundingBox.h"

namespace
{
  // Internal event fired when a contour stroke is completed. Carries the
  // closed contour (in 3D world coords, for persistent display), the small
  // 3D bounding-box-sized uint8 mask (for nnInteractive forwarding), and
  // the corresponding interaction bounding box.
  class LassoContourEvent : public itk::AnyEvent
  {
  public:
    using Self = LassoContourEvent;
    using Superclass = itk::AnyEvent;

    LassoContourEvent() = default;

    LassoContourEvent(mitk::ContourModel* contour,
                       mitk::Image* mask,
                       const mitk::nnInteractive::InteractionBoundingBox& boundingBox)
      : m_Contour(contour), m_Mask(mask), m_BoundingBox(boundingBox)
    {
    }

    LassoContourEvent(const Self& other)
      : Superclass(other),
        m_Contour(other.m_Contour),
        m_Mask(other.m_Mask),
        m_BoundingBox(other.m_BoundingBox)
    {
    }

    ~LassoContourEvent() override = default;

    const char* GetEventName() const override { return "LassoContourEvent"; }

    bool CheckEvent(const itk::EventObject* event) const override
    {
      return dynamic_cast<const Self*>(event) != nullptr;
    }

    itk::EventObject* MakeObject() const override { return new Self(*this); }

    mitk::ContourModel* GetContour() const { return m_Contour; }
    mitk::Image* GetMask() const { return m_Mask; }
    const mitk::nnInteractive::InteractionBoundingBox& GetBoundingBox() const { return m_BoundingBox; }

  private:
    mitk::ContourModel::Pointer m_Contour;
    mitk::Image::Pointer m_Mask;
    mitk::nnInteractive::InteractionBoundingBox m_BoundingBox{};
  };

  // Replaces the AddContourTool wrapper. Builds a live ContourModel in 3D
  // world coordinates while the user drags, renders it via a feedback
  // DataNode, and on release rasterizes into a small bounding-box-sized
  // 3D mask via SegTool2D::WriteSliceToVolume. No 3D working segmentation
  // is involved.
  class LassoContourInteractor : public mitk::EventStateMachine
  {
  public:
    mitkClassMacro(LassoContourInteractor, EventStateMachine)
    itkFactorylessNewMacro(Self)

    void SetReferenceImage(const mitk::Image* image) { m_ReferenceImage = image; }
    void SetDataStorage(mitk::DataStorage* storage) { m_DataStorage = storage; }
    void SetReferenceNode(mitk::DataNode* node) { m_ReferenceNode = node; }
    void SetContourColor(const mitk::Color& color)
    {
      m_ContourColor = color;
      if (m_FeedbackNode.IsNotNull())
        m_FeedbackNode->SetColor(color, nullptr, "contour.color");
    }

    // Drops the live feedback node and any in-flight contour state.
    // Called by the outer Interactor on OnDisable / OnReset.
    void ReleaseFeedback()
    {
      if (m_FeedbackNode.IsNotNull() && m_DataStorage != nullptr && m_DataStorage->Exists(m_FeedbackNode))
        m_DataStorage->Remove(m_FeedbackNode);

      m_FeedbackNode = nullptr;
      m_LiveContour = nullptr;
      m_CurrentPlane = nullptr;
    }

  protected:
    LassoContourInteractor()
    {
      m_ContourColor.Set(0.0f, 1.0f, 0.0f);
    }

    ~LassoContourInteractor() override = default;

    void ConnectActionsAndFunctions() override
    {
      CONNECT_FUNCTION("PrimaryButtonPressed", OnMousePressed);
      CONNECT_FUNCTION("Move", OnMouseMoved);
      CONNECT_FUNCTION("MouseMove", OnHover);
      CONNECT_FUNCTION("Release", OnMouseReleased);
      CONNECT_FUNCTION("InvertLogic", OnInvertLogic);
    }

    // mitk::Tool overrides FilterEvents to skip the DataNode visibility
    // check; we do the same so events flow without us being node-bound.
    bool FilterEvents(mitk::InteractionEvent*, mitk::DataNode*) override
    {
      return true;
    }

    void ConfigurationChanged() override {}

  private:
    void OnMousePressed(mitk::StateMachineAction*, mitk::InteractionEvent* event)
    {
      auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
      if (positionEvent == nullptr || m_ReferenceImage == nullptr)
        return;

      const auto* planeGeometry = positionEvent->GetSender()->GetCurrentWorldPlaneGeometry();
      if (planeGeometry == nullptr)
        return;

      m_CurrentPlane = planeGeometry;
      m_LiveContour = mitk::ContourModel::New();
      m_LiveContour->SetClosed(true);
      m_LiveContour->AddVertex(positionEvent->GetPositionInWorld());

      this->EnsureFeedbackNode();
      m_FeedbackNode->SetData(m_LiveContour);
      m_FeedbackNode->SetVisibility(true);

      this->RequestRendererUpdate(positionEvent);
    }

    void OnMouseMoved(mitk::StateMachineAction*, mitk::InteractionEvent* event)
    {
      auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
      if (positionEvent == nullptr || m_LiveContour.IsNull())
        return;

      m_LiveContour->AddVertex(positionEvent->GetPositionInWorld());
      m_LiveContour->Modified();

      this->RequestRendererUpdate(positionEvent);
    }

    void OnHover(mitk::StateMachineAction*, mitk::InteractionEvent*)
    {
      // No hover preview.
    }

    void OnMouseReleased(mitk::StateMachineAction*, mitk::InteractionEvent* event)
    {
      auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
      if (positionEvent == nullptr || m_LiveContour.IsNull() || m_CurrentPlane.IsNull() ||
          m_ReferenceImage == nullptr)
      {
        this->DiscardCurrentContour();
        return;
      }

      // Reject accidental clicks: a closed lasso needs at least three
      // distinct vertices (matches the behaviour of the previous
      // AddContourTool wrapper).
      if (m_LiveContour->GetNumberOfVertices() < 3)
      {
        this->DiscardCurrentContour();
        this->RequestRendererUpdate(positionEvent);
        return;
      }

      this->FinalizeContour();
      this->RequestRendererUpdate(positionEvent);
    }

    void OnInvertLogic(mitk::StateMachineAction*, mitk::InteractionEvent*)
    {
      // Prompt-type switching is handled externally via the interactor swap.
    }

    void FinalizeContour()
    {
      // Extract a uint8 2D slice of the reference image at the current
      // plane to capture the right extent and world geometry.
      auto refSlice = mitk::SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(
        m_CurrentPlane, m_ReferenceImage, 0.0);
      if (refSlice.IsNull())
      {
        this->DiscardCurrentContour();
        return;
      }

      const auto uint8Type = mitk::MakePixelType<unsigned char, unsigned char, 1>();
      auto paintingSlice = mitk::Image::New();
      paintingSlice->Initialize(uint8Type, *(refSlice->GetTimeGeometry()));
      paintingSlice->AllocateZeroedVolume();

      // Project the 3D world contour into the slice's 2D index space so
      // VTK's stencil (used by FillContourInSlice2) operates in the same
      // frame as the slice's vtkImageData.
      auto projected = mitk::ContourModelUtils::ProjectContourTo2DSlice(paintingSlice, m_LiveContour);
      if (projected.IsNull())
      {
        this->DiscardCurrentContour();
        return;
      }

      mitk::ContourModelUtils::FillContourInSlice2(projected, paintingSlice, 1);

      // Tight stroke bounding box in MITK index space; works for any
      // orientation thanks to the slice-geometry-aware projection.
      mitk::nnInteractive::InteractionBoundingBox boundingBox{};
      const bool haveBoundingBox = mitk::nnInteractive::ComputeStrokeBoundingBox(
        paintingSlice, m_ReferenceImage, boundingBox);

      auto contourCopy = m_LiveContour;
      auto handoffPlane = m_CurrentPlane;
      m_LiveContour = nullptr;
      m_CurrentPlane = nullptr;

      if (m_FeedbackNode.IsNotNull())
      {
        m_FeedbackNode->SetData(nullptr);
        m_FeedbackNode->SetVisibility(false);
      }

      // Degenerate rasterization (e.g. zero-area contour) drops silently.
      if (!haveBoundingBox)
        return;

      auto mask = mitk::nnInteractive::BuildBoundingBoxMaskImage(
        paintingSlice, handoffPlane, m_ReferenceImage, boundingBox);
      if (mask.IsNull())
        return;

      this->InvokeEvent(LassoContourEvent(contourCopy, mask, boundingBox));
    }

    void DiscardCurrentContour()
    {
      m_LiveContour = nullptr;
      m_CurrentPlane = nullptr;
      if (m_FeedbackNode.IsNotNull())
      {
        m_FeedbackNode->SetData(nullptr);
        m_FeedbackNode->SetVisibility(false);
      }
    }

    void EnsureFeedbackNode()
    {
      if (m_FeedbackNode.IsNotNull())
        return;

      m_FeedbackNode = mitk::DataNode::New();
      m_FeedbackNode->SetName("nnInteractive_Lasso_Feedback");
      m_FeedbackNode->SetProperty("helper object", mitk::BoolProperty::New(true));
      m_FeedbackNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
      m_FeedbackNode->SetFloatProperty("contour.width", 3.0f);
      m_FeedbackNode->SetColor(m_ContourColor, nullptr, "contour.color");

      if (m_DataStorage != nullptr)
        m_DataStorage->Add(m_FeedbackNode, m_ReferenceNode);
    }

    void RequestRendererUpdate(const mitk::InteractionPositionEvent* positionEvent)
    {
      if (positionEvent->GetSender() != nullptr && positionEvent->GetSender()->GetRenderWindow() != nullptr)
        mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
    }

    const mitk::Image* m_ReferenceImage = nullptr;
    mitk::DataStorage* m_DataStorage = nullptr;
    mitk::DataNode::Pointer m_ReferenceNode;

    mitk::ContourModel::Pointer m_LiveContour;
    mitk::DataNode::Pointer m_FeedbackNode;
    mitk::PlaneGeometry::ConstPointer m_CurrentPlane;
    mitk::Color m_ContourColor;
  };
}

namespace mitk::nnInteractive
{
  class LassoInteractor::Impl
  {
  public:
    explicit Impl(LassoInteractor* owner)
      : Interactor(LassoContourInteractor::New()),
        m_Owner(owner)
    {
      auto segModule = us::ModuleRegistry::GetModule("MitkSegmentation");
      this->Interactor->LoadStateMachine("PressMoveReleaseWithCTRLInversionAllMouseMoves.xml", segModule);
      this->Interactor->SetEventConfig("SegmentationToolsConfig.xml", segModule);

      auto command = itk::MemberCommand<Impl>::New();
      command->SetCallbackFunction(this, &Impl::OnLassoContourEvent);
      this->Interactor->AddObserver(LassoContourEvent(), command);
    }

    ~Impl() = default;

    bool HasInteractions() const
    {
      for (const auto& [promptType, nodes] : m_LassoNodes)
      {
        if (!nodes.empty())
          return true;
      }
      return false;
    }

    void DestroyLassoNodes()
    {
      auto dataStorage = m_Owner->GetDataStorage();
      for (const auto& [promptType, nodes] : m_LassoNodes)
      {
        if (dataStorage != nullptr)
        {
          for (const auto& node : nodes)
            dataStorage->Remove(node);
        }
      }
      m_LassoNodes.clear();
      m_LastLassoMask = nullptr;
      m_LastLassoBoundingBox.reset();
    }

    LassoContourInteractor::Pointer Interactor;
    std::unordered_map<PromptType, std::vector<DataNode::Pointer>> m_LassoNodes;
    Image::Pointer m_LastLassoMask;
    std::optional<InteractionBoundingBox> m_LastLassoBoundingBox;

  private:
    void OnLassoContourEvent(itk::Object*, const itk::EventObject& event)
    {
      const auto* contourEvent = static_cast<const LassoContourEvent*>(&event);
      auto contour = contourEvent->GetContour();
      auto mask = contourEvent->GetMask();
      if (contour == nullptr || mask == nullptr)
        return;

      const auto promptType = m_Owner->GetCurrentPromptType();

      // Persist the contour as a vector overlay (cheap, renders correctly
      // on any view because it's vector geometry).
      auto node = DataNode::New();
      node->SetData(contour);
      node->SetName(this->CreateLassoNodeName());
      node->SetColor(GetColor(promptType, ColorIntensity::Muted), nullptr, "contour.color");
      node->SetFloatProperty("contour.width", 3.0f);
      node->SetBoolProperty("helper object", true);
      node->SetBoolProperty("includeInBoundingBox", false);

      m_LassoNodes[promptType].push_back(node);
      m_Owner->GetDataStorage()->Add(node, m_Owner->GetToolManager()->GetReferenceData(0));

      m_LastLassoMask = mask;
      m_LastLassoBoundingBox = contourEvent->GetBoundingBox();

      m_Owner->UpdatePreviewEvent(false);
    }

    std::string CreateLassoNodeName()
    {
      const auto& promptType = GetPromptTypeAsString(m_Owner->GetCurrentPromptType());
      return m_Owner->GetDataStorage()->GetUniqueName(promptType + " lasso");
    }

    LassoInteractor* m_Owner;
  };
}

mitk::nnInteractive::LassoInteractor::LassoInteractor()
  : Interactor(InteractionType::Lasso, InteractionMode::BlockLMBDisplayInteraction),
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
  return m_Impl->m_LastLassoMask;
}

const std::array<std::array<int, 2>, 3>* mitk::nnInteractive::LassoInteractor::GetLastLassoBoundingBox() const
{
  return m_Impl->m_LastLassoBoundingBox.has_value() ? &m_Impl->m_LastLassoBoundingBox.value() : nullptr;
}

void mitk::nnInteractive::LassoInteractor::OnSetToolManager()
{
  // State machine is already loaded in Impl's ctor; reference image and
  // data storage are plumbed in OnEnable().
}

void mitk::nnInteractive::LassoInteractor::OnHandleEvent(InteractionEvent* event)
{
  m_Impl->Interactor->HandleEvent(event, nullptr);
}

void mitk::nnInteractive::LassoInteractor::OnEnable()
{
  auto toolManager = this->GetToolManager();
  auto referenceNode = toolManager->GetReferenceData(0);
  if (referenceNode == nullptr)
    return;

  const auto promptType = this->GetCurrentPromptType();
  const auto& color = GetColor(promptType, ColorIntensity::Vibrant);

  m_Impl->Interactor->SetDataStorage(this->GetDataStorage());
  m_Impl->Interactor->SetReferenceNode(referenceNode);
  m_Impl->Interactor->SetReferenceImage(referenceNode->GetDataAs<Image>());
  m_Impl->Interactor->SetContourColor(color);
}

void mitk::nnInteractive::LassoInteractor::OnDisable()
{
  m_Impl->Interactor->ReleaseFeedback();
}

void mitk::nnInteractive::LassoInteractor::OnReset()
{
  m_Impl->Interactor->ReleaseFeedback();
  m_Impl->DestroyLassoNodes();
}
