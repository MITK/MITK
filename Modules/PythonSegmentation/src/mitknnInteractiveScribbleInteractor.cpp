/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveScribbleInteractor.h>

#include <mitkBaseRenderer.h>
#include <mitkContourModel.h>
#include <mitkContourModelUtils.h>
#include <mitkEventStateMachine.h>
#include <mitkExceptionMacro.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkLevelWindowProperty.h>
#include <mitkMatrixConvert.h>
#include <mitkPaintbrushTool.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkSegTool2D.h>
#include <mitkToolManager.h>

#include <usModuleRegistry.h>

#include <memory>
#include <optional>

#include "mitknnInteractiveBoundingBoxHelpers.h"
#include "mitknnInteractiveRenderingHelpers.h"

namespace
{
  // Internal event fired when a brushstroke is completed. Carries the 2D
  // uint8 painting slice, its slicing plane, and the axis-aligned
  // interaction bounding box so the outer ScribbleInteractor can build a
  // small bounding-box-sized 3D mask for forwarding to nnInteractive and
  // for the persistent overlay DataNode.
  class ScribbleStrokeEvent : public itk::AnyEvent
  {
  public:
    using Self = ScribbleStrokeEvent;
    using Superclass = itk::AnyEvent;

    ScribbleStrokeEvent() = default;

    ScribbleStrokeEvent(mitk::Image* slice,
                         const mitk::PlaneGeometry* plane,
                         const mitk::nnInteractive::InteractionBoundingBox& boundingBox)
      : m_Slice(slice), m_Plane(plane), m_BoundingBox(boundingBox)
    {
    }

    ScribbleStrokeEvent(const Self& other)
      : Superclass(other),
        m_Slice(other.m_Slice),
        m_Plane(other.m_Plane),
        m_BoundingBox(other.m_BoundingBox)
    {
    }

    ~ScribbleStrokeEvent() override = default;

    const char* GetEventName() const override { return "ScribbleStrokeEvent"; }

    bool CheckEvent(const itk::EventObject* event) const override
    {
      return dynamic_cast<const Self*>(event) != nullptr;
    }

    itk::EventObject* MakeObject() const override { return new Self(*this); }

    mitk::Image* GetSlice() const { return m_Slice; }
    const mitk::PlaneGeometry* GetPlane() const { return m_Plane; }
    const mitk::nnInteractive::InteractionBoundingBox& GetBoundingBox() const { return m_BoundingBox; }

  private:
    mitk::Image::Pointer m_Slice;
    mitk::PlaneGeometry::ConstPointer m_Plane;
    mitk::nnInteractive::InteractionBoundingBox m_BoundingBox{};
  };

  // The diameter of the brush in pixels of the slice.
  constexpr int BrushSize = 3;

  // A brushstroke, painted directly into a 2D uint8 Image sized to a slice of
  // the reference image along a plane. No 3D working segmentation is
  // required, which eliminates the full-volume ScribbleNode allocation.
  // Brush circle and gap-fill geometry are obtained from
  // PaintbrushTool::CreateBrushContour and CreateGapContour so the two stay
  // in sync. The brush interactor paints strokes along the mouse, and
  // ScribbleInteractor::AddScribble() along given points.
  class BrushStroke
  {
  public:
    // Returns nullptr if the plane does not cut the reference image.
    static std::unique_ptr<BrushStroke> Begin(const mitk::Image* referenceImage,
                                              const mitk::PlaneGeometry* plane,
                                              mitk::TimeStepType timeStep)
    {
      // Extract a slice of the reference image to capture the correct 2D
      // extent and world geometry for the plane. The reference image is
      // already resident (user data), so the only new allocation is the
      // small 2D uint8 painting slice.
      auto refSlice = mitk::SegTool2D::GetAffectedImageSliceAs2DImage(plane, referenceImage, timeStep);
      if (refSlice.IsNull())
        return nullptr;

      return std::unique_ptr<BrushStroke>(new BrushStroke(refSlice, plane));
    }

    // Stamps the brush at a point and fills the gap to the previous one.
    void PaintAt(const mitk::Point3D& worldCoord)
    {
      mitk::Point3D indexCoord;
      m_Slice->GetGeometry()->WorldToIndex(worldCoord, indexCoord);
      indexCoord[0] = std::round(indexCoord[0]);
      indexCoord[1] = std::round(indexCoord[1]);
      indexCoord[2] = 0;

      auto stamp = mitk::ContourModel::New();
      stamp->SetClosed(true);
      for (auto it = m_BrushContour->Begin(); it != m_BrushContour->End(); ++it)
      {
        auto p = (*it)->Coordinates;
        p[0] += indexCoord[0];
        p[1] += indexCoord[1];
        stamp->AddVertex(p);
      }
      mitk::ContourModelUtils::FillContourInSlice2(stamp, m_Slice, 1);

      const double radius = static_cast<double>(BrushSize) / 2.0;
      if (m_LastPosition.has_value() && indexCoord.EuclideanDistanceTo(m_LastPosition.value()) > radius)
      {
        auto gap = mitk::PaintbrushTool::CreateGapContour(m_LastPosition.value(), indexCoord, radius);
        mitk::ContourModelUtils::FillContourInSlice2(gap, m_Slice, 1);
      }

      m_LastPosition = indexCoord;
    }

    // Computes a tight bounding box by mapping the stroke's non-zero 2D
    // footprint through the slice's geometry into the reference's index
    // space. Works for any orientation; on axial/coronal/sagittal it
    // collapses to one voxel along the slicing axis automatically. Empty if
    // nothing was painted inside the image.
    std::optional<mitk::nnInteractive::InteractionBoundingBox> ComputeBoundingBox(const mitk::Image* referenceImage) const
    {
      mitk::nnInteractive::InteractionBoundingBox boundingBox{};

      if (!mitk::nnInteractive::ComputeStrokeBoundingBox(m_Slice, referenceImage, boundingBox))
        return std::nullopt;

      return boundingBox;
    }

    mitk::Image* GetSlice() const { return m_Slice; }
    const mitk::PlaneGeometry* GetPlane() const { return m_Plane; }

  private:
    BrushStroke(const mitk::Image* refSlice, const mitk::PlaneGeometry* plane)
      : m_Slice(mitk::Image::New()),
        // Cloned so that a later change of a renderer-owned plane cannot
        // desync the plane the stroke is written back with.
        m_Plane(plane->Clone()),
        m_BrushContour(mitk::PaintbrushTool::CreateBrushContour(BrushSize))
    {
      m_Slice->Initialize(mitk::MakePixelType<unsigned char, unsigned char, 1>(), *(refSlice->GetTimeGeometry()));
      m_Slice->AllocateZeroedVolume();
    }

    mitk::Image::Pointer m_Slice;
    mitk::PlaneGeometry::ConstPointer m_Plane;
    mitk::ContourModel::Pointer m_BrushContour;
    std::optional<mitk::Point3D> m_LastPosition;
  };

  // Paints brushstrokes along the mouse and shows them while they are drawn.
  class ScribbleBrushInteractor : public mitk::EventStateMachine
  {
  public:
    mitkClassMacro(ScribbleBrushInteractor, EventStateMachine)
    itkFactorylessNewMacro(Self)

    void SetReferenceImage(const mitk::Image* image) { m_ReferenceImage = image; }
    void SetBrushColor(const mitk::Color& color)
    {
      m_BrushColor = color;
      if (m_PaintingNode.IsNotNull())
        m_PaintingNode->SetColor(color);
    }

    void SetDataStorage(mitk::DataStorage* storage) { m_DataStorage = storage; }
    void SetReferenceNode(mitk::DataNode* node) { m_ReferenceNode = node; }

    // Drops the live painting node and any in-flight stroke state. Called
    // by the outer Interactor's OnDisable / OnReset.
    void ReleasePaintingNode()
    {
      if (m_PaintingNode.IsNotNull() && m_DataStorage != nullptr && m_DataStorage->Exists(m_PaintingNode))
        m_DataStorage->Remove(m_PaintingNode);

      m_PaintingNode = nullptr;
      m_Stroke.reset();
    }

  protected:
    ScribbleBrushInteractor()
    {
      m_BrushColor.Set(0.0f, 1.0f, 0.0f);
    }

    ~ScribbleBrushInteractor() override = default;

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
    // paints into its own lazily-created overlay node.
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

      if (!this->BeginStroke(positionEvent))
        return;

      m_PaintingNode->SetVisibility(true);

      m_Stroke->PaintAt(positionEvent->GetPositionInWorld());
      this->RequestRendererUpdate(positionEvent);
    }

    void OnMouseMoved(mitk::StateMachineAction*, mitk::InteractionEvent* event)
    {
      auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
      if (positionEvent == nullptr || m_Stroke == nullptr)
        return;

      // Scrolling to a different slice mid-stroke finalizes the current
      // stroke and starts a new one on the new slice.
      if (this->CheckIfCurrentSliceHasChanged(positionEvent))
      {
        this->FinalizeCurrentStroke();

        if (!this->BeginStroke(positionEvent))
          return;
      }

      m_Stroke->PaintAt(positionEvent->GetPositionInWorld());
      this->RequestRendererUpdate(positionEvent);
    }

    void OnHover(mitk::StateMachineAction*, mitk::InteractionEvent*)
    {
      // No hover preview of the brush circle -- matches current behavior.
    }

    void OnMouseReleased(mitk::StateMachineAction*, mitk::InteractionEvent* event)
    {
      auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
      if (positionEvent == nullptr || m_Stroke == nullptr)
        return;

      this->FinalizeCurrentStroke();
      this->RequestRendererUpdate(positionEvent);
    }

    void OnInvertLogic(mitk::StateMachineAction*, mitk::InteractionEvent*)
    {
      // Prompt-type switching is handled externally (via interactor swap),
      // so CTRL-triggered inversion is a no-op here.
    }

    void FinalizeCurrentStroke()
    {
      if (m_Stroke == nullptr || m_ReferenceImage == nullptr)
        return;

      const auto boundingBox = m_Stroke->ComputeBoundingBox(m_ReferenceImage);
      const auto stroke = std::move(m_Stroke);

      if (m_PaintingNode.IsNotNull())
      {
        m_PaintingNode->SetData(nullptr);
        m_PaintingNode->SetVisibility(false);
      }

      if (boundingBox.has_value())
        this->InvokeEvent(ScribbleStrokeEvent(stroke->GetSlice(), stroke->GetPlane(), boundingBox.value()));
    }

    bool CheckIfCurrentSliceHasChanged(const mitk::InteractionPositionEvent* event)
    {
      const auto* planeGeometry = event->GetSender()->GetCurrentWorldPlaneGeometry();
      if (planeGeometry == nullptr)
        return false;

      if (m_Stroke == nullptr)
        return true;

      const auto* strokePlane = m_Stroke->GetPlane();

      return !mitk::MatrixEqualElementWise(planeGeometry->GetIndexToWorldTransform()->GetMatrix(),
                                            strokePlane->GetIndexToWorldTransform()->GetMatrix()) ||
             !mitk::Equal(planeGeometry->GetIndexToWorldTransform()->GetOffset(),
                          strokePlane->GetIndexToWorldTransform()->GetOffset());
    }

    bool BeginStroke(const mitk::InteractionPositionEvent* event)
    {
      const auto* renderer = event->GetSender();

      m_Stroke = BrushStroke::Begin(m_ReferenceImage, renderer->GetCurrentWorldPlaneGeometry(), renderer->GetTimeStep(m_ReferenceImage));
      if (m_Stroke == nullptr)
        return false;

      this->EnsurePaintingNode();
      m_PaintingNode->SetData(m_Stroke->GetSlice());

      return true;
    }

    void EnsurePaintingNode()
    {
      if (m_PaintingNode.IsNotNull())
        return;

      m_PaintingNode = mitk::DataNode::New();
      m_PaintingNode->SetName("nnInteractive_Scribble_Painting");
      m_PaintingNode->SetProperty("binary", mitk::BoolProperty::New(true));
      m_PaintingNode->SetProperty("outline binary", mitk::BoolProperty::New(false));
      m_PaintingNode->SetProperty("helper object", mitk::BoolProperty::New(true));
      m_PaintingNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
      m_PaintingNode->SetProperty("opacity", mitk::FloatProperty::New(0.8f));
      m_PaintingNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0, 1)));
      m_PaintingNode->SetColor(m_BrushColor);
      mitk::nnInteractive::HideNodeIn3DRenderWindows(m_PaintingNode);

      if (m_DataStorage != nullptr)
        m_DataStorage->Add(m_PaintingNode, m_ReferenceNode);
    }

    void RequestRendererUpdate(const mitk::InteractionPositionEvent* positionEvent)
    {
      if (positionEvent->GetSender() != nullptr && positionEvent->GetSender()->GetRenderWindow() != nullptr)
        mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
    }

    const mitk::Image* m_ReferenceImage = nullptr;
    mitk::DataStorage* m_DataStorage = nullptr;
    mitk::DataNode::Pointer m_ReferenceNode;

    std::unique_ptr<BrushStroke> m_Stroke;
    mitk::DataNode::Pointer m_PaintingNode;
    mitk::Color m_BrushColor;
  };
}

namespace mitk::nnInteractive
{
  class ScribbleInteractor::Impl
  {
  public:
    explicit Impl(ScribbleInteractor* owner)
      : Interactor(ScribbleBrushInteractor::New()),
        m_Owner(owner)
    {
      auto segModule = us::ModuleRegistry::GetModule("MitkSegmentation");
      this->Interactor->LoadStateMachine("PressMoveReleaseWithCTRLInversionAllMouseMoves.xml", segModule);
      this->Interactor->SetEventConfig("SegmentationToolsConfig.xml", segModule);

      auto command = itk::MemberCommand<Impl>::New();
      command->SetCallbackFunction(this, &Impl::OnScribbleStrokeEvent);
      this->Interactor->AddObserver(ScribbleStrokeEvent(), command);
    }

    ~Impl() = default;

    bool HasInteractions() const
    {
      for (const auto& [promptType, nodes] : m_StrokeNodes)
      {
        if (!nodes.empty())
          return true;
      }
      return false;
    }

    void RemoveLastStroke(PromptType promptType)
    {
      auto iter = m_StrokeNodes.find(promptType);

      if (iter == m_StrokeNodes.end() || iter->second.empty())
        return;

      if (auto dataStorage = m_Owner->GetDataStorage(); dataStorage != nullptr)
        dataStorage->Remove(iter->second.back());

      iter->second.pop_back();

      // The cached "last stroke" referred to the stroke just removed; it is
      // only read while applying a fresh stroke, so dropping it is safe.
      m_LastStrokeMask = nullptr;
      m_LastStrokeBoundingBox.reset();
    }

    void DestroyStrokeNodes()
    {
      auto dataStorage = m_Owner->GetDataStorage();
      for (const auto& [promptType, nodes] : m_StrokeNodes)
      {
        if (dataStorage != nullptr)
        {
          for (const auto& node : nodes)
            dataStorage->Remove(node);
        }
      }
      m_StrokeNodes.clear();
      m_LastStrokeMask = nullptr;
      m_LastStrokeBoundingBox.reset();
    }

    // Makes a completed brushstroke the last scribble and shows it. Returns
    // false if it is not inside the reference image.
    bool AddStroke(Image* slice, const PlaneGeometry* plane, const InteractionBoundingBox& boundingBox, PromptType promptType)
    {
      auto referenceNode = m_Owner->GetToolManager()->GetReferenceData(0);
      auto referenceImage = referenceNode != nullptr ? referenceNode->GetDataAs<Image>() : nullptr;
      if (referenceImage == nullptr)
        return false;

      // Build the small 3D bounding-box-sized mask. This serves dual duty:
      // it is forwarded to nnInteractive (as the scribble image alongside
      // interaction_bbox) AND used as the persistent overlay for cross-
      // slice visualization. Using the 3D mask -- not the 2D painting
      // slice -- for persistence ensures correct rendering for oblique
      // planes (the 3D mask carries reference-aligned geometry that MITK
      // can compose on any view).
      auto mask = BuildBoundingBoxMaskImage(slice, plane, referenceImage, boundingBox);
      if (mask.IsNull())
        return false;

      this->AddStrokeNode(mask, promptType, referenceNode);

      m_LastStrokeMask = mask;
      m_LastStrokeBoundingBox = boundingBox;

      return true;
    }

    ScribbleBrushInteractor::Pointer Interactor;
    std::unordered_map<PromptType, std::vector<DataNode::Pointer>> m_StrokeNodes;
    Image::Pointer m_LastStrokeMask;
    std::optional<InteractionBoundingBox> m_LastStrokeBoundingBox;

  private:
    // Persists the mask as an overlay DataNode (visible until OnReset).
    void AddStrokeNode(Image* mask, PromptType promptType, DataNode* referenceNode)
    {
      auto node = DataNode::New();
      node->SetData(mask);
      node->SetName(this->CreateStrokeNodeName(promptType));
      node->SetColor(GetColor(promptType, ColorIntensity::Vibrant));
      node->SetProperty("binary", BoolProperty::New(true));
      node->SetProperty("outline binary", BoolProperty::New(false));
      node->SetProperty("helper object", BoolProperty::New(true));
      node->SetProperty("includeInBoundingBox", BoolProperty::New(false));
      node->SetProperty("opacity", FloatProperty::New(0.65f));
      node->SetProperty("levelwindow", LevelWindowProperty::New(LevelWindow(0, 1)));
      HideNodeIn3DRenderWindows(node);

      m_StrokeNodes[promptType].push_back(node);
      m_Owner->GetDataStorage()->Add(node, referenceNode);
    }

    void OnScribbleStrokeEvent(itk::Object*, const itk::EventObject& event)
    {
      const auto* strokeEvent = static_cast<const ScribbleStrokeEvent*>(&event);
      auto slice = strokeEvent->GetSlice();
      if (slice == nullptr)
        return;

      if (this->AddStroke(slice, strokeEvent->GetPlane(), strokeEvent->GetBoundingBox(), m_Owner->GetCurrentPromptType()))
        m_Owner->UpdatePreviewEvent(false);
    }

    std::string CreateStrokeNodeName(PromptType promptType)
    {
      return m_Owner->GetDataStorage()->GetUniqueName(GetPromptTypeAsString(promptType) + " scribble");
    }

    ScribbleInteractor* m_Owner;
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
  return m_Impl->HasInteractions();
}

mitk::Image::ConstPointer mitk::nnInteractive::ScribbleInteractor::GetLastScribbleMask() const
{
  return m_Impl->m_LastStrokeMask.GetPointer();
}

const mitk::nnInteractive::InteractionBoundingBox* mitk::nnInteractive::ScribbleInteractor::GetLastScribbleBoundingBox() const
{
  return m_Impl->m_LastStrokeBoundingBox.has_value() ? &m_Impl->m_LastStrokeBoundingBox.value() : nullptr;
}

void mitk::nnInteractive::ScribbleInteractor::AddScribble(const PlaneGeometry* plane, const std::vector<Point3D>& points, PromptType promptType)
{
  if (plane == nullptr)
    mitkThrow() << "A scribble needs a plane.";

  auto toolManager = this->GetToolManager();
  auto referenceNode = toolManager != nullptr ? toolManager->GetReferenceData(0) : nullptr;
  const auto* referenceImage = referenceNode != nullptr ? referenceNode->GetDataAs<Image>() : nullptr;

  if (referenceImage == nullptr)
    mitkThrow() << "A scribble needs a reference image.";

  const auto timeStep = referenceImage->GetTimeGeometry()->TimePointToTimeStep(toolManager->GetCurrentTimePoint());

  auto stroke = BrushStroke::Begin(referenceImage, plane, timeStep);
  if (stroke == nullptr)
    mitkThrow() << "The plane of the scribble does not cut the image.";

  for (const auto& point : points)
    stroke->PaintAt(point);

  const auto boundingBox = stroke->ComputeBoundingBox(referenceImage);

  if (!boundingBox.has_value() || !m_Impl->AddStroke(stroke->GetSlice(), stroke->GetPlane(), boundingBox.value(), promptType))
    mitkThrow() << "The scribble does not cover any voxel of the image.";
}

void mitk::nnInteractive::ScribbleInteractor::RemoveLastInteraction(PromptType promptType)
{
  m_Impl->RemoveLastStroke(promptType);
}

void mitk::nnInteractive::ScribbleInteractor::OnHandleEvent(InteractionEvent* event)
{
  m_Impl->Interactor->HandleEvent(event, nullptr);
}

void mitk::nnInteractive::ScribbleInteractor::OnEnable()
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
  m_Impl->Interactor->SetBrushColor(color);
}

void mitk::nnInteractive::ScribbleInteractor::OnDisable()
{
  m_Impl->Interactor->ReleasePaintingNode();
}

void mitk::nnInteractive::ScribbleInteractor::OnReset()
{
  m_Impl->Interactor->ReleasePaintingNode();
  m_Impl->DestroyStrokeNodes();
}
