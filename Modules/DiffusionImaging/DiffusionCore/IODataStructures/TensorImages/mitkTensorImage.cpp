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

#include "mitkTensorImage.h"
#include "mitkImageDataItem.h"
#include "mitkImageCast.h"

#include "itkDiffusionTensor3D.h"
#include "itkTensorToRgbImageFilter.h"

#include "vtkImageData.h"

// #ifdef _OPENMP
// #include "omp.h"
// #endif

mitk::TensorImage::TensorImage() : Image()
{
  m_RgbImage = 0;
}

mitk::TensorImage::~TensorImage()
{

}

vtkImageData* mitk::TensorImage::GetVtkImageData(int t, int n)
{
  if(m_RgbImage.IsNull())
  {
    ConstructRgbImage();
  }
  return m_RgbImage->GetVtkImageData(t,n);
}

const vtkImageData*mitk::TensorImage::GetVtkImageData(int t, int n) const
{
  if(m_RgbImage.IsNull())
  {
    ConstructRgbImage();
  }
  return m_RgbImage->GetVtkImageData(t,n);
}

void mitk::TensorImage::ConstructRgbImage() const
{
    typedef itk::Image<itk::DiffusionTensor3D<float>,3> ImageType;
    typedef itk::TensorToRgbImageFilter<ImageType> FilterType;
    FilterType::Pointer filter = FilterType::New();

    ImageType::Pointer itkvol = ImageType::New();
    mitk::CastToItkImage(this, itkvol);
    filter->SetInput(itkvol);
    filter->Update();

    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( filter->GetOutput() );
    image->SetVolume( filter->GetOutput()->GetBufferPointer() );
    m_RgbImage = image;
}

vtkImageData* mitk::TensorImage::GetNonRgbVtkImageData(int t, int n)
{
  return Superclass::GetVtkImageData(t,n);
}
