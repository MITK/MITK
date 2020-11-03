/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSetRegionTool_h_Included
#define mitkSetRegionTool_h_Included

#include "mitkCommon.h"
#include "mitkFeedbackContourTool.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class Image;
  class StateMachineAction;
  class InteractionEvent;
  /**
    \brief Fills or erases a 2D region

    \sa FeedbackContourTool
    \sa ExtractImageFilter
    \sa OverwriteSliceImageFilter

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Finds the outer contour of a shape in 2D (possibly including holes) and sets all
    the inside pixels to a specified value. This might fill holes or erase segmentations.

    \warning Only to be instantiated by mitk::ToolManager.

    $Author$
  */
  class MITKSEGMENTATION_EXPORT SetRegionTool : public FeedbackContourTool
  {
  public:
    mitkClassMacro(SetRegionTool, FeedbackContourTool);

  protected:
    SetRegionTool(int paintingPixelValue = 1); // purposely hidden
    ~SetRegionTool() override;

    void ConnectActionsAndFunctions() override;

    void Activated() override;
    void Deactivated() override;

    virtual void OnMousePressed(StateMachineAction *, InteractionEvent *);
    virtual void OnMouseReleased(StateMachineAction *, InteractionEvent *);
    virtual void OnMouseMoved(StateMachineAction *, InteractionEvent *);

    int m_PaintingPixelValue;
  };

} // namespace

#endif
