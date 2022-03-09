/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEditbaleContourTool_h_Included
#define mitkEditbaleContourTool_h_Included

#include <mitkFeedbackContourTool.h>

namespace mitk
{

  class MITKSEGMENTATION_EXPORT EditableContourTool : public FeedbackContourTool
  {
  public:
    mitkClassMacro(EditableContourTool, FeedbackContourTool);

    /// \brief Convert all current contours to binary segmentations.
    virtual void ConfirmSegmentation();

    /// \brief Delete all current contours.
    virtual void ClearSegmentation();

  protected:
    EditableContourTool();
    ~EditableContourTool() override;

    void ConnectActionsAndFunctions() override;

    void Activated() override;
    void Deactivated() override;
   
    /// \brief Initialize tool.
    virtual void OnInitContour(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Add a control point and finish current segment.
    virtual void OnAddPoint(StateMachineAction *, InteractionEvent *interactionEvent) = 0;

    /// \brief Draw a contour according to the mouse movement when mouse button is pressed and mouse is moved.
    virtual void OnDrawing(StateMachineAction *, InteractionEvent *interactionEvent) = 0;

    virtual void OnEndDrawing(StateMachineAction *, InteractionEvent *interactionEvent) = 0;

    /// \brief Computation of the preview contour.
    virtual void OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent) = 0;

    /// \brief Finish EditableContour tool.
    virtual void OnFinish(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Finish contour interaction.
    virtual void FinishTool();

    virtual void EnableContourInteraction(bool on) = 0;

    virtual void ReleaseInteractors() = 0;

    virtual void ReleaseHelperObjects();

    virtual void RemoveHelperObjects();

    ContourModel::Pointer CreateNewContour() const;

    virtual void UpdateClosureContour(mitk::Point3D endpoint);

    bool IsPositionEventInsideImageRegion(InteractionPositionEvent *positionEvent, BaseData *data);

    mitk::ContourModel::Pointer m_Contour;
    mitk::DataNode::Pointer m_ContourNode;

    mitk::ContourModel::Pointer m_PreviewContour;
    mitk::DataNode::Pointer m_PreviewContourNode;

    mitk::ContourModel::Pointer m_ClosureContour;
    mitk::DataNode::Pointer m_ClosureContourNode;

    mitk::ContourModel::Pointer m_EditingContour;
    mitk::DataNode::Pointer m_EditingContourNode;

    mitk::ContourModel::Pointer m_CurrentRestrictedArea;
    std::vector<mitk::ContourModel::Pointer> m_RestrictedAreas;

    /** Slice of the reference data the tool is currently actively working on to
    define contours.*/
    mitk::Image::Pointer m_ReferenceDataSlice;

    std::vector<std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer>> m_WorkingContours;
    std::vector<std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer>> m_EditingContours;

    PlaneGeometry::ConstPointer m_PlaneGeometry;  
  };
}

#endif
