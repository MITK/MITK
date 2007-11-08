/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
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

#ifndef mitkSetRegionTool_h_Included
#define mitkSetRegionTool_h_Included

#include "mitkCommon.h"
#include "mitkSegTool2D.h"

namespace mitk
{

class Image;

/**
  \brief Fills or erases a 2D region

  \sa SegTool2D
  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter
  
  \ingroup Interaction
  \ingroup Reliver

  Finds the outer contour of a shape in 2D (possibly including holes) and sets all 
  the inside pixels to a specified value. This might fill holes or erase segmentations.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class SetRegionTool : public SegTool2D
{
  public:
    
    mitkClassMacro(SetRegionTool, SegTool2D);

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

