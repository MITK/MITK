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

#ifndef mitkSetRegionTool_h_Included
#define mitkSetRegionTool_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkFeedbackContourTool.h"

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
    virtual ~SetRegionTool();

    void ConnectActionsAndFunctions() override;

    virtual void Activated() override;
    virtual void Deactivated() override;

    virtual bool OnMousePressed ( StateMachineAction*, InteractionEvent* );
    virtual bool OnMouseReleased( StateMachineAction*, InteractionEvent* );
    virtual bool OnInvertLogic  ( StateMachineAction*, InteractionEvent* );

    int m_PaintingPixelValue;

    bool m_FillContour;
    bool m_StatusFillWholeSlice;

    ContourModel::Pointer m_SegmentationContourInWorldCoordinates;
    ContourModel::Pointer m_WholeImageContourInWorldCoordinates;
};

} // namespace

#endif


