/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourTool_h
#define mitkContourTool_h

#include <mitkCommon.h>
#include <mitkFeedbackContourTool.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  class Image;
  class StateMachineAction;
  class InteractionEvent;

  /**
    \brief Simple contour filling tool.

    \sa FeedbackContourTool
    \sa ExtractImageFilter

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Fills a visible contour (from FeedbackContourTool) during mouse dragging. When the mouse button
    is released, ContourTool tries to extract a slice from the working image and fill in
    the (filled) contour as a binary image.

    The painting "color" is defined by m_PaintingPixelValue, which is set in the constructor
    (by sub-classes) or during some event (e.g. in OnInvertLogic - when CTRL is pressed).

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT ContourTool : public FeedbackContourTool
  {
  public:
    mitkClassMacro(ContourTool, FeedbackContourTool);

  protected:
    ContourTool(int paintingPixelValue = 1); // purposely hidden
    ~ContourTool() override;

    /** \brief Called when the tool is activated. Delegates to superclass. */
    void Activated() override;

    /** \brief Called when the tool is deactivated. Delegates to superclass. */
    void Deactivated() override;

    /** \brief Initialize the feedback contour with the first point on mouse press. */
    virtual void OnMousePressed(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Add subsequent points to the feedback contour during mouse drag. */
    virtual void OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Close the contour, project it to the image slice, and fill it on mouse release. */
    virtual void OnMouseReleased(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Toggle painting pixel value between 0 and 1 (called when CTRL is pressed). */
    virtual void OnInvertLogic(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Wire state machine actions to the corresponding member functions. */
    void ConnectActionsAndFunctions() override;

    /** \brief The pixel value used when filling the contour (0 = erase, 1 = draw). */
    int m_PaintingPixelValue;
  };

} // namespace

#endif
