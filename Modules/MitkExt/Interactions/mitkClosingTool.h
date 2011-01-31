/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 28959 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef MITKClOSINGTOOL_H
#define MITKClOSINGTOOL_H

#include "mitkMorphologicTool.h"
#include "MitkExtExports.h"

//itk
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkGrayscaleMorphologicalClosingImageFilter.h>

#include "mitkClosingTool.xpm"

namespace mitk {

  class MitkExt_EXPORT ClosingTool : public MorphologicTool {

    typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryBallStructuringElement< SegmentationType::PixelType, 3 > BallType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::GrayscaleMorphologicalClosingImageFilter< SegmentationType, SegmentationType, BallType > BallClosingFilterType;
    typedef itk::GrayscaleMorphologicalClosingImageFilter< SegmentationType, SegmentationType, CrossType > CrossClosingFilterType;

  public:
    mitkClassMacro(ClosingTool, MorphologicTool);
    itkNewMacro(ClosingTool);

    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

  protected:

    ClosingTool();
    virtual ~ClosingTool();

    mitk::Image::Pointer ApplyFilter(mitk::Image::Pointer image);

    BallType m_Ball;
    BallClosingFilterType::Pointer m_BallClosingFilter;

    CrossType m_Cross;
    CrossClosingFilterType::Pointer m_CrossClosingFilter;

  };//class


}//namespace

#endif