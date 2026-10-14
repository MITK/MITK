/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRegionGrowingTool_h
#define mitkRegionGrowingTool_h

#include <mitkSegTool2D.h>
#include <mitkWeakPointer.h>
#include <MitkSegmentationExports.h>
#include <itkContinuousIndex.h>
#include <itkImage.h>
#include <string>
#include <vector>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief A slice based region growing tool.

    \sa SegTool2D

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Pressing the mouse button into a 2D slice sets a seed. Dragging with the button held grows a region from the
    seed and moves its edge to the cursor: the region contains all pixels at most as far from the seed, in geodesic
    distance, as the pixel under the cursor. Releasing the button writes the region into the active label of the
    working segmentation. A click without dragging writes nothing.

    The geodesic distance makes stepping onto a pixel cost more the less its gray value matches the tissue grown so
    far, whose mean and standard deviation are re-estimated during growth. A leak into a neighboring tissue therefore
    has to pay for every dissimilar pixel it crosses, so it creeps in gradually instead of flooding everything behind
    a single gap.

    Dragging with Ctrl held draws a cut line instead. Once the line separates parts of the active label in the slice,
    the part that would be removed is shown, and releasing the button removes it. The part containing the last seed
    is kept if its region was grown in this slice and label, otherwise the largest part. Holding Ctrl already
    prepares the cut before the button is pressed: the cursor changes, the segmentation is dimmed and the last seed
    is marked.

    Pressing Escape or the secondary mouse button while the button is held discards the preview or the cut.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT RegionGrowingTool : public SegTool2D
  {
  public:
    mitkClassMacro(RegionGrowingTool, SegTool2D);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    const char *GetName() const override;

    /** \brief Intensity region-growing from a seed determines the affected pixels, so SEMIAUTOMATIC. */
    mitk::Label::AlgorithmType GetAlgorithmType() const override;

  protected:
    RegionGrowingTool(); // purposely hidden
    ~RegionGrowingTool() override;

    void ConnectActionsAndFunctions() override;

    void Activated() override;
    void Deactivated() override;

    /** \brief Discards the preview and the cut, as they belong to the previous time point. */
    void OnTimePointChanged() override;

  private:
    /** \brief Called when the user presses the button. Sets the seed and computes the geodesic distances from it. */
    void OnMousePressed(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Called when the mouse moves with the button held. Moves the region's edge to the cursor, once the mouse
     * left a small deadzone around the seed. */
    void OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Called when the user releases the button. Writes the region into the active label if the mouse was
     * dragged. */
    void OnMouseReleased(StateMachineAction *, InteractionEvent *);

    /** \brief Called when Ctrl is held. Changes the cursor, dims the segmentation in the event's render window and
     * marks the last seed if a cut there keeps its side. */
    void OnCutArmed(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Called when Ctrl is no longer held. Undoes OnCutArmed(). */
    void OnCutDisarmed(StateMachineAction *, InteractionEvent *);

    /** \brief Called when the user presses the button with Ctrl held. Starts a cut line. */
    void OnCutStarted(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Called when the mouse moves while cutting. Extends the cut line and shows what it would remove. */
    void OnCutMoved(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Called when the user releases the button while cutting. Removes what the cut line separated. */
    void OnCutFinished(StateMachineAction *, InteractionEvent *);

    /** \brief Called when the user presses Escape or the secondary button while the button is held. Discards the
     * preview or the cut without writing it. */
    void OnCancel(StateMachineAction *, InteractionEvent *);

    using BinaryImageType = itk::Image<DefaultSegmentationDataType, 2>;

    void UpdatePreview();
    void UpdatePreviewLayers();

    /** \brief Returns whether the preview could be written. */
    bool WriteBackPreview();
    void DiscardPreview();

    void UpdateCut();
    void DiscardCut();

    void ArmCut(const BaseRenderer* renderer);
    void DisarmCut();

    /** \brief Dims the working segmentation in the armed render window only. */
    void DimSegmentation();
    void RestoreSegmentationOpacity();

    /** \brief Whether a cut in renderer keeps the side of the last seed: its region was written into the active label
     * of the working segmentation, in the slice and at the time point renderer shows. */
    bool IsLastSeedApplicable(const BaseRenderer* renderer) const;

    Image::Pointer m_WorkingSlice;
    PlaneGeometry::Pointer m_WorkingPlane;
    TimeStepType m_TimeStep = 0;
    DataNode::Pointer m_PreviewNode;
    DataNode::Pointer m_PreviewOutlineNode;

    /** Geodesic distance of every pixel of the slice from the seed, x running fastest. */
    std::vector<double> m_Distances;
    BinaryImageType::SizeType m_SliceSize = {};
    BinaryImageType::SpacingType m_SliceSpacing;

    itk::Index<2> m_SeedIndex = {};
    Point3D m_SeedInWorld;

    /** The distance up to which the region grows, set by the cursor while dragging. */
    double m_MaxDistance = 0;

    Point2D m_PressPositionOnScreen;
    bool m_IsDragging = false;

    /** The seed of the last region written and where it was written to, see IsLastSeedApplicable(). */
    Point3D m_LastSeed;
    PlaneGeometry::Pointer m_LastSeedPlane;
    TimeStepType m_LastSeedTimeStep = 0;
    WeakPointer<MultiLabelSegmentation> m_LastSeedSegmentation;
    MultiLabelSegmentation::LabelValueType m_LastSeedLabelValue = 0;

    /** The slice of the active label's group a cut works on, and the cut line drawn so far. */
    Image::Pointer m_CutSlice;
    PlaneGeometry::Pointer m_CutPlane;
    TimeStepType m_CutTimeStep = 0;
    DefaultSegmentationDataType m_CutLabelValue = 0;
    BinaryImageType::Pointer m_CutLabelMask;
    BinaryImageType::Pointer m_CutLabelParts;
    std::size_t m_CutNumberOfLabelParts = 0;
    BinaryImageType::Pointer m_CutLineMask;
    Image::Pointer m_CutAway;
    itk::ContinuousIndex<double, 2> m_CutLastIndex;
    itk::Index<2> m_CutStartIndex = {};
    itk::ContinuousIndex<double, 2> m_ProtectedPoint;
    bool m_HasProtectedPoint = false;
    DataNode::Pointer m_CutLineNode;
    DataNode::Pointer m_CutAwayNode;

    /** Marks the last seed while cutting, as its side of the cut is kept. */
    DataNode::Pointer m_SeedMarkerNode;

    /** While Ctrl is held, the segmentation is dimmed in the armed render window so the image along the cut stays
     * visible. The window is kept by name, as it may be destroyed while armed. */
    bool m_IsCutArmed = false;
    std::string m_ArmedRendererName;
    DataNode::Pointer m_DimmedNode;
  };

} // namespace

#endif
