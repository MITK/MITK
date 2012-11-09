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
#ifndef MITKOPENINGTOOL_H
#define MITKOPENINGTOOL_H

#include "mitkMorphologicTool.h"
#include "MitkExtExports.h"

//itk
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkGrayscaleMorphologicalOpeningImageFilter.h>

#include "mitkOpeningTool.xpm"

namespace mitk {

  class MitkExt_EXPORT OpeningTool : public MorphologicTool {

    typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryBallStructuringElement< SegmentationType::PixelType, 3 > BallType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::GrayscaleMorphologicalOpeningImageFilter<SegmentationType, SegmentationType, BallType > BallOpeningFilterType;
    typedef itk::GrayscaleMorphologicalOpeningImageFilter< SegmentationType, SegmentationType, CrossType > CrossOpeningFilterType;

  public:
    mitkClassMacro(OpeningTool, MorphologicTool);
    itkNewMacro(OpeningTool);

    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

  protected:

    OpeningTool();
    virtual ~OpeningTool();

    mitk::Image::Pointer ApplyFilter(mitk::Image::Pointer image);

    BallType m_Ball;
    BallOpeningFilterType::Pointer m_BallOpeningFilter;

    CrossType m_Cross;
    CrossOpeningFilterType::Pointer m_CrossOpeningFilter;

  };//class


}//namespace

#endif
