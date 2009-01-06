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

#ifndef mitkCorrectorTool2D_h_Included
#define mitkCorrectorTool2D_h_Included

#include "mitkCommon.h"
#include "mitkFeedbackContourTool.h"

namespace mitk
{

class Image;

/**
  \brief Corrector tool for 2D binary segmentations

  \sa FeedbackContourTool
  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter
  
  \ingroup Interaction
  \ingroup Reliver

  Lets the user draw a (multi-point) line and intelligently decides what to do. The underlying algorithm
  tests if the line begins and ends inside or outside a segmentation and either adds or subtracts a piece of
  segmentation.

  Algorithm is implemented in CorrectorAlgorithm (so that it could be reimplemented in a more modern fashion some time).

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MITK_CORE_EXPORT CorrectorTool2D : public FeedbackContourTool
{
  public:
    
    mitkClassMacro(CorrectorTool2D, FeedbackContourTool);
    itkNewMacro(CorrectorTool2D);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:
    
    CorrectorTool2D(int paintingPixelValue = 1); // purposely hidden
    virtual ~CorrectorTool2D();

    virtual void Activated();
    virtual void Deactivated();
    
    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseMoved   (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);

    int m_PaintingPixelValue;

    Image::Pointer m_WorkingSlice;
};

} // namespace

#endif


