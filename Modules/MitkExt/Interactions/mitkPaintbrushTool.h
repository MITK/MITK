/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkPaintbrushTool_h_Included
#define mitkPaintbrushTool_h_Included

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkFeedbackContourTool.h"
#include "mitkPointSet.h"
#include "mitkPointOperation.h"

namespace mitk
{
 /**
  \brief Paintbrush tool for InteractiveSegmentation

  \sa FeedbackContourTool
  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter
  
  \ingroup Interaction
  \ingroup Reliver

  Simple paintbrush drawing tool. Right now there are only circular pens of varying size.

  \todo Bug #1727: Should be modified, so that the contour is always visible, i.e.without pressing a mouse button.
  
  \warning Only to be instantiated by mitk::ToolManager.
  $Author: maleike $
*/
class MitkExt_EXPORT PaintbrushTool : public FeedbackContourTool
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

    int m_PaintingPixelValue;
    static int m_Size;

    Contour::Pointer m_MasterContour;

    int m_LastContourSize;
};

} // namespace

#endif

