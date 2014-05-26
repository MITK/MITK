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

#ifndef mitkFeedbackContourTool_h_Included
#define mitkFeedbackContourTool_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkSegTool2D.h"
#include "mitkContourModelUtils.h"
#include "mitkContourUtils.h" //TODO remove legacy support
#include "mitkImage.h"

#include "mitkDataNode.h"

#include "mitkImageCast.h"

namespace mitk
{

/**
  \brief Base class for tools that use a contour for feedback

  \sa Tool
  \sa ContourModel

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Implements helper methods, that might be of use to all kind of 2D segmentation tools that use a contour for user feedback.
   - Providing a feedback contour that might be added or removed from the visible scene (SetFeedbackContourVisible).
   - Filling of a contour into a 2D slice

   These helper methods are actually implemented in ContourUtils now. FeedbackContourTool only forwards such requests.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author: nolden $
*/
class MitkSegmentation_EXPORT FeedbackContourTool : public SegTool2D
{
  public:

    mitkClassMacro(FeedbackContourTool, SegTool2D);

  protected:

    FeedbackContourTool(); // purposely hidden
    FeedbackContourTool(const char*); // purposely hidden
    virtual ~FeedbackContourTool();

    ContourModel* GetFeedbackContour();
    void SetFeedbackContour(ContourModel&);

    void Disable3dRendering();
    void SetFeedbackContourVisible(bool);

    /// Provide values from 0.0 (black) to 1.0 (full color)
    void SetFeedbackContourColor( float r, float g, float b );
    void SetFeedbackContourColorDefault();

    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

      \param correctionForIpSegmentation adds 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    ContourModel::Pointer ProjectContourTo2DSlice(Image* slice, ContourModel* contourIn3D, bool correctionForIpSegmentation = false, bool constrainToInside = true);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.

      \param correctionForIpSegmentation subtracts 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry* sliceGeometry, ContourModel* contourIn2D, bool correctionForIpSegmentation = false);

    /**
      \brief Fill a contour in a 2D slice with a specified pixel value.
    */
    void FillContourInSlice( ContourModel* projectedContour, Image* sliceImage, int paintingPixelValue = 1 );

    /**
      \brief Fill a contour in a 2D slice with a specified pixel value at a given time step.
    */
    void FillContourInSlice( ContourModel* projectedContour, unsigned int timeStep, Image* sliceImage, int paintingPixelValue = 1 );

  private:

    ContourModel::Pointer      m_FeedbackContour;
    DataNode::Pointer m_FeedbackContourNode;
    bool                  m_FeedbackContourVisible;

};

} // namespace

#endif

