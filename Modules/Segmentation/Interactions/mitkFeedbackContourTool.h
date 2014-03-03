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
#include "SegmentationExports.h"
#include "mitkSegTool2D.h"
#include "mitkContourModel.h"
#include "mitkImage.h"

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
class Segmentation_EXPORT FeedbackContourTool : public SegTool2D
{
  public:

    mitkClassMacro(FeedbackContourTool, SegTool2D);

  protected:

    FeedbackContourTool(); // purposely hidden
    FeedbackContourTool(const char*); // purposely hidden
    virtual ~FeedbackContourTool();

    virtual void Activated();
    virtual void Deactivated();

    ContourModel* GetFeedbackContour();
    void SetFeedbackContour(ContourModel&);

    void Disable3DRendering();
    void SetFeedbackContourVisible(bool);

    /**
      \brief Sets the color to the feedback contour.
    */
    void SetFeedbackContourColor( float r, float g, float b );

    /**
      \brief Sets the default color to the feedback contour.
    */
    void SetFeedbackContourColorDefault();

  private:

    ContourModel::Pointer   m_FeedbackContour;
    DataNode::Pointer       m_FeedbackContourNode;
    bool                    m_FeedbackContourVisible;

};

} // namespace

#endif
