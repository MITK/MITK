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

#ifndef mitkCorrectorTool2D_h_Included
#define mitkCorrectorTool2D_h_Included

#include "mitkCommon.h"
#include "mitkSegTool2D.h"

#include "ipSegmentation.h"

namespace mitk
{

class Image;

/**
  \brief Corrector tool for 2D binary segmentations

  \sa SegTool2D
  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter
  
  \ingroup Interaction
  \ingroup Reliver

  Lets the user draw a (multi-point) line and intelligently decides what to do. The underlying algorithm
  tests if the line begins and ends inside or outside a segmentation and either adds or substracts a piece of
  segmentation.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class CorrectorTool2D : public SegTool2D
{
  public:
    
    mitkClassMacro(CorrectorTool2D, SegTool2D);
    itkNewMacro(CorrectorTool2D);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:
    
    typedef struct 
    {
      int  lineStart;
      int lineEnd;
      bool modified;
    } 
    TSegData;

    CorrectorTool2D(int paintingPixelValue = 1); // purposely hidden
    virtual ~CorrectorTool2D();

    virtual void Activated();
    virtual void Deactivated();
    
    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseMoved   (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);

    /**
      \brief The algorithm is described in full length in Tobias Heimann's diploma thesis.
    */
    void TobiasHeimannCorrectionAlgorithm(ipPicDescriptor* pic);

    /**
      \brief helper for TobiasHeimannCorrectionAlgorithm
    */
    bool modifySegment( int lineStart, int lineEnd, ipMITKSegmentationTYPE state, ipPicDescriptor *pic, int* _ofsArray );

    int m_PaintingPixelValue;

    Image::Pointer m_WorkingSlice;
};

} // namespace

#endif

