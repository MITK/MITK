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
#include <mitkExceptionMacro.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkProperties.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkSegTool2D.h>
#include <mitkToolManager.h>

#include <usModuleRegistry.h>

#include <optional>

#include "mitknnInteractiveBoundingBoxHelpers.h"
#include "mitknnInteractiveRenderingHelpers.h"

namespace
{
  // Aligns a single-time-step ContourModel's time bounds with a time step of
  // the reference image. Without this, ContourModel::New() produces a
  // default time geometry that only covers time point 0, so the contour
  // stays invisible at any other time step of a 4D reference image.
  void AlignContourToTimeStep(mitk::ContourModel* contour,
                              const mitk::Image* referenceImage,
                              mitk::TimeStepType timeStep)
  {
    if (contour == nullptr || referenceImage == nullptr)
      return;

    const auto* refTimeGeometry = referenceImage->GetTimeGeometry();
    if (refTimeGeometry == nullptr)
      return;

    if (!refTimeGeometry->IsValidTimeStep(timeStep))
      return;

    const auto firstTimePoint = refTimeGeometry->GetMinimumTimePoint(timeStep);
    const auto stepDuration = refTimeGeometry->GetMaximumTimePoint(timeStep) - firstTimePoint;

    auto contourTimeGeometry = mitk::ProportionalTimeGeometry::New();
    contourTimeGeometry->Initialize(contour->GetGeometry(), 1);
    contourTimeGeometry->SetFirstTimePoint(firstTimePoint);
    contourTimeGeometry->SetStepDuration(stepDuration);

    contour->SetTimeGeometry(contourTimeGeometry);
  }

  // Internal event fired when a contour stroke is completed. Carries the
  // closed contour (in 3D world coords, for persistent display), the 2D
  // uint8 painting slice and its slicing plane (for the outer Impl to
  // build the bbox-sized mask), and the corresponding interaction bounding
  // box. Mirrors ScribbleStrokeEvent so both interactors keep mask
  // construction outside the inner state machine.
  class LassoContourEvent : public itk::AnyEvent
  {
  public:
    using Self = LassoContourEvent;
    using Superclass = itk::AnyEvent;

    LassoContourEvent() = default;

    LassoContourEvent(mitk::ContourModel* contour,
                       mitk::Image* slice,
                       const mitk::PlaneGeometry* plane,
                       const mitk::nnInteractive::InteractionBoundingBox& boundingBox)
      : m_Contour(contour), m_Slice(slice), m_Plane(plane), m_BoundingBox(boundingBox)
    {
    }

    LassoContourEvent(const Self& other)
      : Superclass(other),
        m_Contour(other.m_Contour),
        m_Slice(other.m_Slice),
        m_Plane(other.m_Plane),
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
    mitk::Image* GetSlice() const { return m_Slice; }
    const mitk::PlaneGeometry* GetPlane() const { return m_Plane; }
    const mitk::nnInteractive::InteractionBoundingBox& GetBoundingBox() const { return m_BoundingBox; }

  private:
    mitk::ContourModel::Pointer m_Contour;
    mitk::Image::Pointer m_Slice;
    mitk::PlaneGeometry::ConstPointer m_Plane;
    mitk::nnInteractive::InteractionBoundingBox m_BoundingBox{};
  };

  // A closed contour filled into a 2D uint8 slice of the reference image.
  struct FilledContour
  {
    mitk::Image::Pointer Slice;
    mitk::nnInteractive::InteractionBoundingBox BoundingBox{};
  };

  // Fills a closed contour into a slice of the reference image along the
  // plane it is drawn in. The contour interactor fills the contours drawn
  // with the mouse, and LassoInteractor::AddLasso() those of given points.
  // Empty if no pixel of the image is inside, as for a contour without area.
  std::optional<FilledContour> FillContour(const mitk::ContourModel* contour,
                                           const mitk::PlaneGeometry* plane,
                                           mitk::TimeStepType timeStep,
                                           const mitk::Image* referenceImage)
  {
    auto refSlice = mitk::SegTool2D::GetAffectedImageSliceAs2DImage(plane, referenceImage, timeStep);
    if (refSlice.IsNull())
      return std::nullopt;

    const auto uint8Type = mitk::MakePixelType<unsigned char, unsigned char, 1>();
    auto paintingSlice = mitk::Image::New();
    paintingSlice->Initialize(uint8Type, *(refSlice->GetTimeGeometry()));
    paintingSlice->AllocateZeroedVolume();

    // Project the 3D world contour into the slice's 2D index space so
    // VTK's stencil (used by FillContourInSlice2) operates in the same
    // frame as the slice's vtkImageData.
    auto projected = mitk::ContourModelUtils::ProjectContourTo2DSlice(paintingSlice, contour);
    if (projected.IsNull())
      return std::nullopt;

    mitk::ContourModelUtils::FillContourInSlice2(projected, paintingSlice, 1);

    // Tight stroke bounding box in MITK index space; works for any
    // orientation thanks to the slice-geometry-aware projection.
    FilledContour filled{ paintingSlice };

    if (!mitk::nnInteractive::ComputeStrokeBoundingBox(paintingSlice, referenceImage, filled.BoundingBox))
      return std::nullopt;

    return filled;
  }

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
        m_FeedbackNode->SetColor(color);
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

    // Restrict to 2D render windows (drawing requires a slicing plane) and
    // bypass the base-class DataNode visibility check, since this interactor
    // owns its own lazily-created feedback node.
    bool FilterEvents(mitk::InteractionEvent* event, mitk::DataNode*) override
    {
      return event != nullptr &&
             event->GetSender() != nullptr &&
             event->GetSender()->GetMapperID() == mitk::BaseRenderer::Standard2D;
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

      // Clone the renderer-owned plane so a mid-stroke mutation by the
      // renderer cannot desync the plane we hand to WriteSliceToVolume on
      // release. The 2D-only restriction on the state machine already makes
      // mid-stroke navigation unlikely; this is cheap defence in depth.
      m_CurrentPlane = planeGeometry->Clone();
      m_LiveContour = mitk::ContourModel::New();
      m_LiveContour->SetClosed(true);
      m_LiveContour->AddVertex(positionEvent->GetPositionInWorld());

      // Align the (single-time-step) contour's time bounds with the time
      // step the renderer is currently displaying. Without this, the
      // contour's default time bounds cover only time point 0 and it stays
      // invisible whenever the user views any other time step of a 4D
      // reference image.
      AlignContourToTimeStep(m_LiveContour, m_ReferenceImage,
                             positionEvent->GetSender()->GetTimeStep(m_ReferenceImage));

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

      this->FinalizeContour(positionEvent);
      this->RequestRendererUpdate(positionEvent);
    }

    void OnInvertLogic(mitk::StateMachineAction*, mitk::InteractionEvent*)
    {
      // Prompt-type switching is handled externally via the interactor swap.
    }

    void FinalizeContour(const mitk::InteractionPositionEvent* positionEvent)
    {
      // The renderer's time step picks the correct time step for 4D data.
      const auto filled = FillContour(m_LiveContour, m_CurrentPlane,
        positionEvent->GetSender()->GetTimeStep(m_ReferenceImage), m_ReferenceImage);

      const auto contourCopy = m_LiveContour;
      const auto handoffPlane = m_CurrentPlane;
      this->DiscardCurrentContour();

      // Degenerate rasterization (e.g. zero-area contour) drops silently.
      if (!filled.has_value())
        return;

      // Hand off slice + plane + bbox to the outer Impl, which builds the
      // bbox-sized 3D mask. Mirrors ScribbleStrokeEvent.
      this->InvokeEvent(LassoContourEvent(contourCopy, filled->Slice, handoffPlane, filled->BoundingBox));
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
      m_FeedbackNode->SetColor(m_ContourColor);
      mitk::nnInteractive::HideNodeIn3DRenderWindows(m_FeedbackNode);

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

    void RemoveLastLasso(PromptType promptType)
    {
      auto iter = m_LassoNodes.find(promptType);

      if (iter == m_LassoNodes.end() || iter->second.empty())
        return;

      if (auto dataStorage = m_Owner->GetDataStorage(); dataStorage != nullptr)
        dataStorage->Remove(iter->second.back());

      iter->second.pop_back();

      // The cached "last lasso" referred to the contour just removed; it is
      // only read while applying a fresh contour, so dropping it is safe.
      m_LastLassoMask = nullptr;
      m_LastLassoBoundingBox.reset();
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

    // Makes a filled contour the last lasso and shows it. Returns false if
    // it is not inside the reference image.
    bool AddContour(ContourModel* contour, Image* slice, const PlaneGeometry* plane, const InteractionBoundingBox& boundingBox, PromptType promptType)
    {
      auto referenceNode = m_Owner->GetToolManager()->GetReferenceData(0);
      auto referenceImage = referenceNode != nullptr ? referenceNode->GetDataAs<Image>() : nullptr;
      if (referenceImage == nullptr)
        return false;

      // Build the small 3D bounding-box-sized mask here (in the outer Impl)
      // rather than inside the inner state machine, mirroring the Scribble
      // path so both interactors keep mask construction symmetrical.
      auto mask = BuildBoundingBoxMaskImage(slice, plane, referenceImage, boundingBox);
      if (mask.IsNull())
        return false;

      this->AddLassoNode(contour, promptType, referenceNode);

      m_LastLassoMask = mask;
      m_LastLassoBoundingBox = boundingBox;

      return true;
    }

    LassoContourInteractor::Pointer Interactor;
    std::unordered_map<PromptType, std::vector<DataNode::Pointer>> m_LassoNodes;
    Image::Pointer m_LastLassoMask;
    std::optional<InteractionBoundingBox> m_LastLassoBoundingBox;

  private:
    // Persists the contour as a vector overlay (cheap, renders correctly on
    // any view because it's vector geometry).
    void AddLassoNode(ContourModel* contour, PromptType promptType, DataNode* referenceNode)
    {
      auto node = DataNode::New();
      node->SetData(contour);
      node->SetName(this->CreateLassoNodeName(promptType));
      node->SetColor(GetColor(promptType, ColorIntensity::Muted));
      node->SetFloatProperty("contour.width", 3.0f);
      node->SetBoolProperty("helper object", true);
      node->SetBoolProperty("includeInBoundingBox", false);
      HideNodeIn3DRenderWindows(node);

      m_LassoNodes[promptType].push_back(node);
      m_Owner->GetDataStorage()->Add(node, referenceNode);
    }

    void OnLassoContourEvent(itk::Object*, const itk::EventObject& event)
    {
      const auto* contourEvent = static_cast<const LassoContourEvent*>(&event);
      auto contour = contourEvent->GetContour();
      auto slice = contourEvent->GetSlice();
      if (contour == nullptr || slice == nullptr)
        return;

      if (this->AddContour(contour, slice, contourEvent->GetPlane(), contourEvent->GetBoundingBox(), m_Owner->GetCurrentPromptType()))
        m_Owner->UpdatePreviewEvent(false);
    }

    std::string CreateLassoNodeName(PromptType promptType)
    {
      return m_Owner->GetDataStorage()->GetUniqueName(GetPromptTypeAsString(promptType) + " lasso");
    }

    LassoInteractor* m_Owner;
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

mitk::Image::ConstPointer mitk::nnInteractive::LassoInteractor::GetLastLassoMask() const
{
  return m_Impl->m_LastLassoMask.GetPointer();
}

const mitk::nnInteractive::InteractionBoundingBox* mitk::nnInteractive::LassoInteractor::GetLastLassoBoundingBox() const
{
  return m_Impl->m_LastLassoBoundingBox.has_value() ? &m_Impl->m_LastLassoBoundingBox.value() : nullptr;
}

void mitk::nnInteractive::LassoInteractor::AddLasso(const PlaneGeometry* plane, const std::vector<Point3D>& points, PromptType promptType)
{
  if (plane == nullptr)
    mitkThrow() << "A lasso needs a plane.";

  auto toolManager = this->GetToolManager();
  auto referenceNode = toolManager != nullptr ? toolManager->GetReferenceData(0) : nullptr;
  const auto* referenceImage = referenceNode != nullptr ? referenceNode->GetDataAs<Image>() : nullptr;

  if (referenceImage == nullptr)
    mitkThrow() << "A lasso needs a reference image.";

  const auto timeStep = referenceImage->GetTimeGeometry()->TimePointToTimeStep(toolManager->GetCurrentTimePoint());

  // As the contour interactor builds one from the positions of the mouse,
  // which lie in the plane. The render windows draw a contour only close to
  // their plane, and it is filled only within half a voxel of it.
  auto contour = ContourModel::New();
  contour->SetClosed(true);

  for (const auto& point : points)
  {
    Point3D projected;
    plane->Project(point, projected);
    contour->AddVertex(projected);
  }

  AlignContourToTimeStep(contour, referenceImage, timeStep);

  const auto filled = FillContour(contour, plane, timeStep, referenceImage);

  if (!filled.has_value() || !m_Impl->AddContour(contour, filled->Slice, plane, filled->BoundingBox, promptType))
    mitkThrow() << "The contour does not enclose any voxel of the image.";
}

void mitk::nnInteractive::LassoInteractor::RemoveLastInteraction(PromptType promptType)
{
  m_Impl->RemoveLastLasso(promptType);
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
