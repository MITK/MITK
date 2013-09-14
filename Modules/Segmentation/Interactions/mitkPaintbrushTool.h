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

#ifndef mitkPaintbrushTool_h_Included
#define mitkPaintbrushTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkFeedbackContourTool.h"
#include "mitkPointSet.h"
#include "mitkPointOperation.h"
#include "mitkLegacyAdaptors.h"

namespace mitk
{
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
class Segmentation_EXPORT PaintbrushTool : public FeedbackContourTool
{
  public:

    // sent when the pen size is changed or should be updated in a GUI.
    Message1<int>     SizeChanged;

    mitkClassMacro(PaintbrushTool, FeedbackContourTool);

    void SetSize(int value);

  protected:

    PaintbrushTool(int paintingPixelValue = 1); // purposely hidden
    virtual ~PaintbrushTool();

    virtual void Activated();
    virtual void Deactivated();

    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseMoved   (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);
    virtual bool OnInvertLogic  (Action*, const StateEvent*);

    /**
     * \todo This is a possible place where to introduce
     *       different types of pens
     */
    void UpdateContour(const StateEvent* stateEvent);


    /**
    *   Little helper function. Returns the upper left corner of the given pixel.
    */
    mitk::Point2D upperLeft(mitk::Point2D p);

    /**
      * Checks  if the current slice has changed
      */
    void CheckIfCurrentSliceHasChanged(const PositionEvent* event);

    int m_PaintingPixelValue;
    static int m_Size;

    ContourModel::Pointer m_MasterContour;

    int m_LastContourSize;

    Image::Pointer m_WorkingSlice;
    PlaneGeometry::Pointer m_CurrentPlane;
    DataNode::Pointer m_WorkingNode;
};

} // namespace

#endif

