/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkLiveWireTool2D_h
#define mitkLiveWireTool2D_h

#include <mitkSegTool2D.h>
#include <mitkContourModelLiveWireInteractor.h>

namespace mitk
{
  /**
    \brief A 2D segmentation tool based on a LiveWire approach.

    The contour between the last point and the current mouse position is
    computed by searching the shortest path according to specific features of
    the image. The contour thus tends to snap to the boundary of objects.

    \sa SegTool2D
    \sa ImageLiveWireContourModelFilter

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT LiveWireTool2D : public SegTool2D
  {
  public:
    mitkClassMacro(LiveWireTool2D, SegTool2D)
    itkFactorylessNewMacro(Self)

    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;
    const char *GetName() const override;
    const char **GetXPM() const override;

    /// \brief Convert all current contours to binary segmentations.
    void ConfirmSegmentation();

    /// \brief Delete all current contours.
    void ClearSegmentation();

  protected:
    LiveWireTool2D();
    ~LiveWireTool2D() override;

    void ConnectActionsAndFunctions() override;
    void Activated() override;
    void Deactivated() override;

  private:
    /// \brief Initialize tool.
    void OnInitLiveWire(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Add a control point and finish current segment.
    void OnAddPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Actual LiveWire computation.
    void OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Check double click on first control point to finish the LiveWire tool.
    bool OnCheckPoint(const InteractionEvent *interactionEvent);

    /// \brief Finish LiveWire tool.
    void OnFinish(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Close the contour.
    void OnLastSegmentDelete(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Don't use dynamic cost map for LiveWire calculation.
    void OnMouseMoveNoDynamicCosts(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Finish contour interaction.
    void FinishTool();

    void EnableContourLiveWireInteraction(bool on);

    bool IsPositionEventInsideImageRegion(InteractionPositionEvent *positionEvent, BaseData *data);

    void ReleaseInteractors();

    void ReleaseHelperObjects();

    void RemoveHelperObjects();

    template <typename TPixel, unsigned int VImageDimension>
    void FindHighestGradientMagnitudeByITK(itk::Image<TPixel, VImageDimension> *inputImage,
                                           itk::Index<3> &index,
                                           itk::Index<3> &returnIndex);

    mitk::ContourModel::Pointer m_Contour;
    mitk::DataNode::Pointer m_ContourNode;

    mitk::ContourModel::Pointer m_LiveWireContour;
    mitk::DataNode::Pointer m_LiveWireContourNode;

    mitk::ContourModel::Pointer m_EditingContour;
    mitk::DataNode::Pointer m_EditingContourNode;
    mitk::ContourModelLiveWireInteractor::Pointer m_ContourInteractor;

    mitk::Image::Pointer m_WorkingSlice;

    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;

    bool m_CreateAndUseDynamicCosts;

    std::vector<std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer>> m_WorkingContours;
    std::vector<std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer>> m_EditingContours;
    std::vector<mitk::ContourModelLiveWireInteractor::Pointer> m_LiveWireInteractors;

    PlaneGeometry::ConstPointer m_PlaneGeometry;
  };
}

#endif
