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


#ifndef __mitkTensorImage__h
#define __mitkTensorImage__h

#include "mitkImage.h"
#include "itkVectorImage.h"

#include "MitkDiffusionImagingExports.h"


#define TENSOR_NUM_ELEMENTS 6

namespace mitk
{

  /**
  * \brief this class encapsulates tensor images
  */
  class MitkDiffusionImaging_EXPORT TensorImage : public Image
  {

  public:

    mitkClassMacro( TensorImage, Image );
    itkNewMacro(Self);

    virtual ImageVtkAccessor* GetNonRgbVtkImageData(int t = 0, int n = 0);

    virtual ImageVtkAccessor* GetVtkImageData(int t = 0, int n = 0);

    virtual void ConstructRgbImage();

  protected:
    TensorImage();
    virtual ~TensorImage();

    mitk::Image::Pointer m_RgbImage;

  };

} // namespace mitk

#endif /* __mitkTensorImage__h */
