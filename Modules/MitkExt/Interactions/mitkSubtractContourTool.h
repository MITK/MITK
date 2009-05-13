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

#ifndef mitkSubtractContourTool_h_Included
#define mitkSubtractContourTool_h_Included

#include "mitkContourTool.h"

namespace mitk
{

/**
  \brief Fill the inside of a contour with 1

  \sa ContourTool
  
  \ingroup Interaction
  \ingroup Reliver

  Fills a visible contour (from FeedbackContourTool) during mouse dragging. When the mouse button
  is released, SubtractContourTool tries to extract a slice from the working image and fill in
  the (filled) contour as a binary image. All inside pixels are set to 0.
  
  While holding the CTRL key, the contour changes color and the pixels on the inside would be
  filled with 1.


  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MITKEXT_CORE_EXPORT SubtractContourTool : public ContourTool
{
  public:
    
    mitkClassMacro(SubtractContourTool, ContourTool);
    itkNewMacro(SubtractContourTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:
    
    SubtractContourTool(); // purposely hidden
    virtual ~SubtractContourTool();

};

} // namespace

#endif


