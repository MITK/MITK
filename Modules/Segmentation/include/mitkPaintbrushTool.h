/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPaintbrushTool_h
#define mitkPaintbrushTool_h

#include <mitkCommon.h>
#include <mitkFeedbackContourTool.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  class StateMachineAction;
  class InteractionEvent;
  class InteractionPositionEvent;

  /**
   \brief Paintbrush tool for InteractiveSegmentation

   \sa FeedbackContourTool
   \sa ExtractSliceFilter

   \ingroup Interaction
   \ingroup ToolManagerEtAl

   Simple paintbrush drawing tool. Right now there are only circular pens of varying size.


   \warning Only to be instantiated by mitk::ToolManager.
   $Author: maleike $
 */
  class MITKSEGMENTATION_EXPORT PaintbrushTool : public FeedbackContourTool
  {
  public:
    /** \brief Emitted when the pen size changes or should be updated in a GUI. */
    Message1<int> SizeChanged;

    mitkClassMacro(PaintbrushTool, FeedbackContourTool);

    /**
     * \brief Sets the radius of the circular paintbrush pen.
     * \param[in] value The pen radius in pixels.
     */
    void SetSize(int value);

    /**
     * \brief Builds a closed circular brush contour of the given size in 2D
     *        index coordinates, centered at the origin.
     *
     * Half-pixel corner correction is applied so that even and odd sizes
     * stamp symmetrically around the index where the brush is rendered.
     *
     * \pre size > 0 (a non-positive size yields a degenerate contour).
     */
    static ContourModel::Pointer CreateBrushContour(int size);

    /**
     * \brief Builds a closed four-vertex rectangle ContourModel that fills the
     *        gap swept between two brush positions, in 2D index coordinates.
     *
     * Used to avoid holes when the mouse moves more than one brush radius
     * between successive samples.
     *
     * \pre from != to (a zero-length vector causes the internal
     *      direction.normalize() to produce NaNs).
     */
    static ContourModel::Pointer CreateGapContour(const Point3D& from,
                                                  const Point3D& to,
                                                  double radius);

  protected:
    PaintbrushTool(bool startWithFillMode = true); // purposely hidden
    ~PaintbrushTool() override;

    void ConnectActionsAndFunctions() override;

    void Activated() override;
    void Deactivated() override;

    virtual void OnMousePressed(StateMachineAction *, InteractionEvent *);
    virtual void OnMouseMoved(StateMachineAction *, InteractionEvent *);
    virtual void OnPrimaryButtonPressedMoved(StateMachineAction *, InteractionEvent *);
    virtual void MouseMoved(mitk::InteractionEvent *interactionEvent, bool leftMouseButtonPressed);
    virtual void OnMouseReleased(StateMachineAction *, InteractionEvent *);
    void UpdateFeedbackColor();
    virtual void OnInvertLogic(StateMachineAction *, InteractionEvent *);

    virtual int GetFillValue() const;

    void UpdateContour(const InteractionPositionEvent *);

    /**
    *   Little helper function. Returns the upper left corner of the given pixel.
    */
    static mitk::Point2D upperLeft(mitk::Point2D p);

    /**
      * Checks  if the current slice has changed and updates (if needed m_CurrentPlane).
      */
    bool CheckIfCurrentSliceHasChanged(const InteractionPositionEvent *event);

    void ResetWorkingSlice(const InteractionPositionEvent* event);

    void OnToolManagerWorkingDataModified();

    bool m_FillMode;
    const bool m_InitialFillMode;
    int m_Size;

    ContourModel::Pointer m_MasterContour;

    int m_LastContourSize;

    Image::Pointer m_WorkingSlice;
    Image::Pointer m_PaintingSlice;
    PlaneGeometry::ConstPointer m_CurrentPlane;
    DataNode::Pointer m_PaintingNode;
    mitk::Point3D m_LastPosition;

  };

} // namespace

#endif
