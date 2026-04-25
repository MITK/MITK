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
#include <mitkInteractionPositionEvent.h>
#include <mitkLevelWindowProperty.h>
#include <mitkMatrixConvert.h>
#include <mitkPaintbrushTool.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkSegTool2D.h>
#include <mitkToolManager.h>

#include <usModuleRegistry.h>

#include "mitknnInteractiveBoundingBoxHelpers.h"

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

  // Paints brushstrokes directly into a 2D uint8 Image sized to the current
  // slicing plane of the reference image. Replaces the former
  // DrawPaintbrushTool wrapper -- no 3D working segmentation is required,
  // which eliminates the full-volume ScribbleNode allocation. Brush circle
  // and gap-fill geometry are obtained from PaintbrushTool::CreateBrushContour
  // and CreateGapContour so the two stay in sync.
  class ScribbleBrushInteractor : public mitk::EventStateMachine
  {
  public:
    mitkClassMacro(ScribbleBrushInteractor, EventStateMachine)
    itkFactorylessNewMacro(Self)

    void SetReferenceImage(const mitk::Image* image) { m_ReferenceImage = image; }
    void SetBrushSize(int size) { m_Size = size; }
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
      m_PaintingSlice = nullptr;
      m_CurrentPlane = nullptr;
      m_MasterContour = nullptr;
      m_LastContourSize = 0;
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

    // Mirrors mitk::Tool::FilterEvents: dispatch regardless of the DataNode
    // passed to HandleEvent, since this interactor paints into its own
    // lazily-created overlay node rather than an externally-bound one.
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

      if (!this->ResetPaintingSlice(positionEvent))
        return;

      m_PaintingSlice->GetGeometry()->WorldToIndex(positionEvent->GetPositionInWorld(), m_LastPosition);
      m_PaintingNode->SetVisibility(true);

      this->PaintBrushAt(positionEvent, true);
      this->RequestRendererUpdate(positionEvent);
    }

    void OnMouseMoved(mitk::StateMachineAction*, mitk::InteractionEvent* event)
    {
      auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
      if (positionEvent == nullptr || m_PaintingSlice.IsNull())
        return;

      // Scrolling to a different slice mid-stroke finalizes the current
      // stroke and starts a new one on the new slice.
      if (this->CheckIfCurrentSliceHasChanged(positionEvent))
      {
        this->FinalizeCurrentStroke();

        if (!this->ResetPaintingSlice(positionEvent))
          return;

        m_PaintingSlice->GetGeometry()->WorldToIndex(positionEvent->GetPositionInWorld(), m_LastPosition);
      }

      this->PaintBrushAt(positionEvent, true);
      this->RequestRendererUpdate(positionEvent);
    }

    void OnHover(mitk::StateMachineAction*, mitk::InteractionEvent*)
    {
      // No hover preview of the brush circle -- matches current behavior.
    }

    void OnMouseReleased(mitk::StateMachineAction*, mitk::InteractionEvent* event)
    {
      auto* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
      if (positionEvent == nullptr || m_PaintingSlice.IsNull())
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
      if (m_PaintingSlice.IsNull() || m_CurrentPlane.IsNull() || m_ReferenceImage == nullptr)
        return;

      // Compute a tight bounding box by mapping the stroke's non-zero 2D
      // footprint through the slice's geometry into the reference's index
      // space. Works for any orientation; on axial/coronal/sagittal it
      // collapses to one voxel along the slicing axis automatically.
      mitk::nnInteractive::InteractionBoundingBox boundingBox{};
      const bool haveBoundingBox = mitk::nnInteractive::ComputeStrokeBoundingBox(
        m_PaintingSlice, m_ReferenceImage, boundingBox);

      auto handoffSlice = m_PaintingSlice;
      auto handoffPlane = m_CurrentPlane;
      m_PaintingSlice = nullptr;

      if (m_PaintingNode.IsNotNull())
      {
        m_PaintingNode->SetData(nullptr);
        m_PaintingNode->SetVisibility(false);
      }
      m_CurrentPlane = nullptr;

      if (haveBoundingBox)
        this->InvokeEvent(ScribbleStrokeEvent(handoffSlice, handoffPlane, boundingBox));
    }

    bool CheckIfCurrentSliceHasChanged(const mitk::InteractionPositionEvent* event)
    {
      const auto* planeGeometry = event->GetSender()->GetCurrentWorldPlaneGeometry();
      if (planeGeometry == nullptr)
        return false;

      if (m_CurrentPlane.IsNull() || m_PaintingSlice.IsNull())
        return true;

      return !mitk::MatrixEqualElementWise(planeGeometry->GetIndexToWorldTransform()->GetMatrix(),
                                            m_CurrentPlane->GetIndexToWorldTransform()->GetMatrix()) ||
             !mitk::Equal(planeGeometry->GetIndexToWorldTransform()->GetOffset(),
                          m_CurrentPlane->GetIndexToWorldTransform()->GetOffset());
    }

    bool ResetPaintingSlice(const mitk::InteractionPositionEvent* event)
    {
      const auto* planeGeometry = event->GetSender()->GetCurrentWorldPlaneGeometry();
      if (planeGeometry == nullptr || m_ReferenceImage == nullptr)
        return false;

      // Extract a slice of the reference image to capture the correct 2D
      // extent and world geometry for the current plane. The reference image
      // is already resident (user data), so the only new allocation is the
      // small 2D uint8 painting slice.
      auto refSlice = mitk::SegTool2D::GetAffectedImageSliceAs2DImage(event, m_ReferenceImage);
      if (refSlice.IsNull())
        return false;

      const auto uint8Type = mitk::MakePixelType<unsigned char, unsigned char, 1>();
      m_PaintingSlice = mitk::Image::New();
      m_PaintingSlice->Initialize(uint8Type, *(refSlice->GetTimeGeometry()));
      m_PaintingSlice->AllocateZeroedVolume();

      m_CurrentPlane = planeGeometry;

      this->EnsurePaintingNode();
      m_PaintingNode->SetData(m_PaintingSlice);

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

      if (m_DataStorage != nullptr)
        m_DataStorage->Add(m_PaintingNode, m_ReferenceNode);
    }

    void UpdateMasterContourIfNeeded()
    {
      if (m_LastContourSize == m_Size && m_MasterContour.IsNotNull())
        return;

      m_MasterContour = mitk::PaintbrushTool::CreateBrushContour(m_Size);
      m_LastContourSize = m_Size;
    }

    void PaintBrushAt(const mitk::InteractionPositionEvent* positionEvent, bool buttonPressed)
    {
      if (!buttonPressed)
        return;

      this->UpdateMasterContourIfNeeded();

      mitk::Point3D worldCoord = positionEvent->GetPositionInWorld();
      mitk::Point3D indexCoord;
      m_PaintingSlice->GetGeometry()->WorldToIndex(worldCoord, indexCoord);
      indexCoord[0] = std::round(indexCoord[0]);
      indexCoord[1] = std::round(indexCoord[1]);
      indexCoord[2] = 0;

      auto stamp = mitk::ContourModel::New();
      stamp->SetClosed(true);
      for (auto it = m_MasterContour->Begin(); it != m_MasterContour->End(); ++it)
      {
        auto p = (*it)->Coordinates;
        p[0] += indexCoord[0];
        p[1] += indexCoord[1];
        stamp->AddVertex(p);
      }
      mitk::ContourModelUtils::FillContourInSlice2(stamp, m_PaintingSlice, 1);

      const double dist = indexCoord.EuclideanDistanceTo(m_LastPosition);
      const double radius = static_cast<double>(m_Size) / 2.0;
      if (dist > radius)
      {
        auto gap = mitk::PaintbrushTool::CreateGapContour(m_LastPosition, indexCoord, radius);
        mitk::ContourModelUtils::FillContourInSlice2(gap, m_PaintingSlice, 1);
      }

      m_LastPosition = indexCoord;
    }

    void RequestRendererUpdate(const mitk::InteractionPositionEvent* positionEvent)
    {
      if (positionEvent->GetSender() != nullptr && positionEvent->GetSender()->GetRenderWindow() != nullptr)
        mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
    }

    const mitk::Image* m_ReferenceImage = nullptr;
    mitk::DataStorage* m_DataStorage = nullptr;
    mitk::DataNode::Pointer m_ReferenceNode;

    mitk::Image::Pointer m_PaintingSlice;
    mitk::DataNode::Pointer m_PaintingNode;
    mitk::PlaneGeometry::ConstPointer m_CurrentPlane;
    mitk::ContourModel::Pointer m_MasterContour;
    mitk::Point3D m_LastPosition{};

    int m_Size = 3;
    int m_LastContourSize = 0;
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

    ScribbleBrushInteractor::Pointer Interactor;
    std::unordered_map<PromptType, std::vector<DataNode::Pointer>> m_StrokeNodes;
    Image::Pointer m_LastStrokeMask;
    std::optional<InteractionBoundingBox> m_LastStrokeBoundingBox;

  private:
    void OnScribbleStrokeEvent(itk::Object*, const itk::EventObject& event)
    {
      const auto* strokeEvent = static_cast<const ScribbleStrokeEvent*>(&event);
      auto slice = strokeEvent->GetSlice();
      if (slice == nullptr)
        return;

      auto referenceNode = m_Owner->GetToolManager()->GetReferenceData(0);
      auto referenceImage = referenceNode != nullptr ? referenceNode->GetDataAs<Image>() : nullptr;
      if (referenceImage == nullptr)
        return;

      // Build the small 3D bounding-box-sized mask. This serves dual duty:
      // it is forwarded to nnInteractive (as the scribble image alongside
      // interaction_bbox) AND used as the persistent overlay for cross-
      // slice visualization. Using the 3D mask -- not the 2D painting
      // slice -- for persistence ensures correct rendering for oblique
      // planes (the 3D mask carries reference-aligned geometry that MITK
      // can compose on any view).
      auto mask = BuildBoundingBoxMaskImage(slice, strokeEvent->GetPlane(), referenceImage, strokeEvent->GetBoundingBox());
      if (mask.IsNull())
        return;

      // Persist the mask as an overlay DataNode (visible until OnReset).
      const auto promptType = m_Owner->GetCurrentPromptType();
      auto node = DataNode::New();
      node->SetData(mask);
      node->SetName(this->CreateStrokeNodeName());
      node->SetColor(GetColor(promptType, ColorIntensity::Vibrant));
      node->SetProperty("binary", BoolProperty::New(true));
      node->SetProperty("outline binary", BoolProperty::New(false));
      node->SetProperty("helper object", BoolProperty::New(true));
      node->SetProperty("includeInBoundingBox", BoolProperty::New(false));
      node->SetProperty("opacity", FloatProperty::New(0.65f));
      node->SetProperty("levelwindow", LevelWindowProperty::New(LevelWindow(0, 1)));

      m_StrokeNodes[promptType].push_back(node);
      m_Owner->GetDataStorage()->Add(node, referenceNode);

      m_LastStrokeMask = mask;
      m_LastStrokeBoundingBox = strokeEvent->GetBoundingBox();

      m_Owner->UpdatePreviewEvent(false);
    }

    std::string CreateStrokeNodeName()
    {
      const auto& promptType = GetPromptTypeAsString(m_Owner->GetCurrentPromptType());
      return m_Owner->GetDataStorage()->GetUniqueName(promptType + " scribble");
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

const mitk::Image* mitk::nnInteractive::ScribbleInteractor::GetLastScribbleMask() const
{
  return m_Impl->m_LastStrokeMask;
}

const mitk::nnInteractive::InteractionBoundingBox* mitk::nnInteractive::ScribbleInteractor::GetLastScribbleBoundingBox() const
{
  return m_Impl->m_LastStrokeBoundingBox.has_value() ? &m_Impl->m_LastStrokeBoundingBox.value() : nullptr;
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
  m_Impl->Interactor->SetBrushSize(3);
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
