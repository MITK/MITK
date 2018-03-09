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

#include <MitkDiffusionCoreExports.h>


#define TENSOR_NUM_ELEMENTS 6

namespace mitk
{

  /**
  * \brief this class encapsulates tensor images
  */
  class MITKDIFFUSIONCORE_EXPORT TensorImage : public Image
  {

  public:

    typedef float ScalarPixelType;
    typedef itk::DiffusionTensor3D<ScalarPixelType> PixelType;
    typedef itk::Image<PixelType,3> ItkTensorImageType;

    mitkClassMacro( TensorImage, Image )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual vtkImageData* GetNonRgbVtkImageData(int t = 0, int n = 0);

    vtkImageData* GetVtkImageData(int t = 0, int n = 0) override;
    const vtkImageData* GetVtkImageData(int t = 0, int n = 0) const override;

    virtual void ConstructRgbImage() const;

  protected:
    TensorImage();
    ~TensorImage() override;

    mutable mitk::Image::Pointer m_RgbImage;

  };

} // namespace mitk

#endif /* __mitkTensorImage__h */
