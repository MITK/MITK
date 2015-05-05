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
#include <MitkSegmentationExports.h>
#include "mitkFeedbackContourTool.h"
#include "mitkPointSet.h"
#include "mitkPointOperation.h"
#include "mitkLegacyAdaptors.h"

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
    Message1<int>     SizeChanged;

    mitkClassMacro(PaintbrushTool, FeedbackContourTool);

    void SetSize(int value);

  protected:

    PaintbrushTool(int paintingPixelValue = 1); // purposely hidden
    virtual ~PaintbrushTool();

    void ConnectActionsAndFunctions() override;

    virtual void Activated() override;
    virtual void Deactivated() override;

    virtual bool OnMousePressed ( StateMachineAction*, InteractionEvent* );
    virtual bool OnMouseMoved   ( StateMachineAction*, InteractionEvent* );
    virtual bool OnPrimaryButtonPressedMoved( StateMachineAction*, InteractionEvent* );
    virtual bool MouseMoved(mitk::InteractionEvent* interactionEvent, bool leftMouseButtonPressed);
    virtual bool OnMouseReleased( StateMachineAction*, InteractionEvent* );
    virtual bool OnInvertLogic  ( StateMachineAction*, InteractionEvent* );

    /**
     * \todo This is a possible place where to introduce
     *       different types of pens
     */
    void UpdateContour( const InteractionPositionEvent* );


    /**
    *   Little helper function. Returns the upper left corner of the given pixel.
    */
    mitk::Point2D upperLeft(mitk::Point2D p);

    /**
      * Checks  if the current slice has changed
      */
    void CheckIfCurrentSliceHasChanged(const InteractionPositionEvent* event);

    void OnToolManagerWorkingDataModified();

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

