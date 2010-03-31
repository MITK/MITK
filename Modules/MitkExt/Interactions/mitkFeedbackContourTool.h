/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkFeedbackContourTool_h_Included
#define mitkFeedbackContourTool_h_Included

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkSegTool2D.h"
#include "mitkContour.h"
#include "mitkContourUtils.h"
#include "mitkImage.h"

#include "mitkDataNode.h"

#include "mitkImageCast.h"

namespace mitk
{

/**
  \brief Base class for tools that use a contour for feedback

  \sa Tool
  \sa Contour

  \ingroup Interaction
  \ingroup Reliver

  Implements helper methods, that might be of use to all kind of 2D segmentation tools that use a contour for user feedback. 
   - Providing a feedback contour that might be added or removed from the visible scene (SetFeedbackContourVisible).
   - Filling of a contour into a 2D slice

   These helper methods are actually implemented in ContourUtils now. FeedbackContourTool only forwards such requests.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author: nolden $
*/
class MitkExt_EXPORT FeedbackContourTool : public SegTool2D
{
  public:
    
    mitkClassMacro(FeedbackContourTool, SegTool2D);

  protected:

    FeedbackContourTool(); // purposely hidden
    FeedbackContourTool(const char*); // purposely hidden
    virtual ~FeedbackContourTool();

    Contour* GetFeedbackContour();
    void SetFeedbackContour(Contour&);
    void SetFeedbackContourVisible(bool);

    /// Provide values from 0.0 (black) to 1.0 (full color)
    void SetFeedbackContourColor( float r, float g, float b );
    void SetFeedbackContourColorDefault();

    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

      \param correctionForIpSegmentation adds 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    Contour::Pointer ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool correctionForIpSegmentation = false, bool constrainToInside = true);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.
      
      \param correctionForIpSegmentation subtracts 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    Contour::Pointer BackProjectContourFrom2DSlice(Image* slice, Contour* contourIn2D, bool correctionForIpSegmentation = false);

    /**
      \brief Fill a contour in a 2D slice with a specified pixel value.
    */
    void FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue = 1 );

  private:

    Contour::Pointer      m_FeedbackContour;
    DataNode::Pointer m_FeedbackContourNode;
    bool                  m_FeedbackContourVisible;

    ContourUtils::Pointer m_ContourUtils;
};

} // namespace

#endif

