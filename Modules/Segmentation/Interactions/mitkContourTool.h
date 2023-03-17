/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourTool_h
#define mitkContourTool_h

#include "mitkCommon.h"
#include "mitkFeedbackContourTool.h"
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

    $Author$
  */
  class MITKSEGMENTATION_EXPORT ContourTool : public FeedbackContourTool
  {
  public:
    mitkClassMacro(ContourTool, FeedbackContourTool);

  protected:
    ContourTool(int paintingPixelValue = 1); // purposely hidden
    ~ContourTool() override;

    void Activated() override;
    void Deactivated() override;

    virtual void OnMousePressed(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void OnMouseReleased(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void OnInvertLogic(StateMachineAction *, InteractionEvent *interactionEvent);

    void ConnectActionsAndFunctions() override;

    int m_PaintingPixelValue;
  };

} // namespace

#endif
