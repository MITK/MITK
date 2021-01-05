/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPaintbrushTool_h_Included
#define mitkPaintbrushTool_h_Included

#include "mitkCommon.h"
#include "mitkFeedbackContourTool.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class StateMachineAction;
  class InteractionEvent;
  class InteractionPositionEvent;

  /**
   \brief Paintbrush tool for InteractiveSegmentation

   \sa FeedbackContourTool
   \sa ExtractImageFilter
   \sa OverwriteSliceImageFilter

   \ingroup Interaction
   \ingroup ToolManagerEtAl

   Simple paintbrush drawing tool. Right now there are only circular pens of varying size.


   \warning Only to be instantiated by mitk::ToolManager.
   $Author: maleike $
 */
  class MITKSEGMENTATION_EXPORT PaintbrushTool : public FeedbackContourTool
  {
  public:
    // sent when the pen size is changed or should be updated in a GUI.
    Message1<int> SizeChanged;

    mitkClassMacro(PaintbrushTool, FeedbackContourTool);

    void SetSize(int value);

  protected:
    PaintbrushTool(int paintingPixelValue = 1); // purposely hidden
    ~PaintbrushTool() override;

    void ConnectActionsAndFunctions() override;

    void Activated() override;
    void Deactivated() override;

    virtual void OnMousePressed(StateMachineAction *, InteractionEvent *);
    virtual void OnMouseMoved(StateMachineAction *, InteractionEvent *);
    virtual void OnPrimaryButtonPressedMoved(StateMachineAction *, InteractionEvent *);
    virtual void MouseMoved(mitk::InteractionEvent *interactionEvent, bool leftMouseButtonPressed);
    virtual void OnMouseReleased(StateMachineAction *, InteractionEvent *);
    virtual void OnInvertLogic(StateMachineAction *, InteractionEvent *);

    /**
     * \todo This is a possible place where to introduce
     *       different types of pens
     */
    void UpdateContour(const InteractionPositionEvent *);

    /**
    *   Little helper function. Returns the upper left corner of the given pixel.
    */
    mitk::Point2D upperLeft(mitk::Point2D p);

    /**
      * Checks  if the current slice has changed
      */
    void CheckIfCurrentSliceHasChanged(const InteractionPositionEvent *event);

    void OnToolManagerWorkingDataModified();

    int m_PaintingPixelValue;
    static int m_Size;

    ContourModel::Pointer m_MasterContour;

    int m_LastContourSize;

    Image::Pointer m_WorkingSlice;
    PlaneGeometry::ConstPointer m_CurrentPlane;
    DataNode::Pointer m_WorkingNode;
    mitk::Point3D m_LastPosition;
  };

} // namespace

#endif
