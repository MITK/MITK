/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkContourTool_h_Included
#define mitkContourTool_h_Included

#include "mitkCommon.h"
#include "mitkFeedbackContourTool.h"

namespace mitk
{

class Image;

/**
  \brief Simple contour filling tool.

  \sa FeedbackContourTool
  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter
  
  \ingroup Interaction
  \ingroup Reliver

  Fills a visible contour (from FeedbackContourTool) during mouse dragging. When the mouse button
  is released, ContourTool tries to extract a slice from the working image and fill in
  the (filled) contour as a binary image.

  The painting "color" is defined by m_PaintingPixelValue, which is set in the constructor
  (by sub-classes) or during some event (e.g. in OnInvertLogic - when CTRL is pressed). 

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MITKEXT_CORE_EXPORT ContourTool : public FeedbackContourTool
{
  public:
    
    mitkClassMacro(ContourTool, FeedbackContourTool);

  protected:
   
    ContourTool(int paintingPixelValue = 1); // purposely hidden
    virtual ~ContourTool();

    virtual void Activated();
    virtual void Deactivated();
    
    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseMoved   (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);
    virtual bool OnInvertLogic  (Action*, const StateEvent*);


    int m_PaintingPixelValue;
};

} // namespace

#endif


