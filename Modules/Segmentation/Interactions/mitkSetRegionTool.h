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
#include "SegmentationExports.h"
#include "mitkFeedbackContourTool.h"

namespace mitk
{

class Image;

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
class Segmentation_EXPORT SetRegionTool : public FeedbackContourTool
{
  public:

    mitkClassMacro(SetRegionTool, FeedbackContourTool);

  protected:

    SetRegionTool(int paintingPixelValue = 1); // purposely hidden
    virtual ~SetRegionTool();

    virtual void Activated();
    virtual void Deactivated();

    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);
    virtual bool OnInvertLogic  (Action*, const StateEvent*);

    int m_PaintingPixelValue;

    bool m_FillContour;
    bool m_StatusFillWholeSlice;

    Contour::Pointer m_SegmentationContourInWorldCoordinates;
    Contour::Pointer m_WholeImageContourInWorldCoordinates;
};

} // namespace

#endif


