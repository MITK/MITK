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
#ifndef __mitkConvert2Dto3DImageFilter_h
#define __mitkConvert2Dto3DImageFilter_h

//MITK
#include <mitkImage.h>
#include "mitkImageToImageFilter.h"
#include <itkImage.h>

namespace mitk
{

  class MITK_CORE_EXPORT Convert2Dto3DImageFilter : public ImageToImageFilter
  {
  public: 

    mitkClassMacro( Convert2Dto3DImageFilter , ImageToImageFilter );
    itkNewMacro( Self );

  protected:
    /*!
    \brief standard constructor
    */
    Convert2Dto3DImageFilter();
    /*!
    \brief standard destructor
    */
    ~Convert2Dto3DImageFilter();
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the smoothed output image.
    */
    virtual void GenerateData();

    /*!
    \brief Make a 2D image to a 3D image
    */
    template<typename TPixel, unsigned int VImageDimension>
    void ItkConvert2DTo3D( itk::Image<TPixel,VImageDimension>* itkImage, mitk::Image::Pointer &mitkImage );

  };
} //END mitk namespace
#endif
