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

#include "mitkShImage.h"
#include "mitkImageCast.h"
#include "itkImage.h"
#include "mitkImageVtkAccessor.h"
#include <mitkProperties.h>
#include "itkShToRgbImageFilter.h"

mitk::ShImage::ShImage() : Image()
  , m_ShOrder(0)
  , m_NumCoefficients(0)
{
  m_RgbImage = nullptr;
  // not needed anymore as soon as all diffusion images are identified via properties anyway
  this->SetProperty("IsShImage", mitk::BoolProperty::New(true));
}

mitk::ShImage::~ShImage()
{

}

unsigned int mitk::ShImage::NumCoefficients()
{
  if (m_NumCoefficients==0 || m_ShOrder==0)
  {
    m_NumCoefficients = static_cast<unsigned int>(this->m_ImageDescriptor->GetChannelTypeById(0).GetNumberOfComponents());
    int c=3, d=2-2*static_cast<int>(m_NumCoefficients);
    int D = c*c-4*d;
    if (D>0)
    {
      int s = (-c+static_cast<int>(sqrt(D)))/2;
      if (s<0)
        s = (-c-static_cast<int>(sqrt(D)))/2;
      m_ShOrder = static_cast<unsigned int>(s);
    }
    else if (D==0)
      m_ShOrder = static_cast<unsigned int>(-c/2);
  }
  return m_NumCoefficients;
}

unsigned int mitk::ShImage::ShOrder()
{
  NumCoefficients();
  return m_ShOrder;
}

vtkImageData* mitk::ShImage::GetVtkImageData(int t, int n)
{
  if(m_RgbImage.IsNull())
  {
    ShOrder();
    NumCoefficients();
    ConstructRgbImage();
  }
   return m_RgbImage->GetVtkImageData(t,n);
}

const vtkImageData*mitk::ShImage::GetVtkImageData(int t, int n) const
{
  if(m_RgbImage.IsNull())
  {
    ConstructRgbImage();
  }
  return m_RgbImage->GetVtkImageData(t,n);
}

template<int nShOrder>
void mitk::ShImage::Construct() const
{
  typedef itk::Image<itk::Vector<float, ( nShOrder * nShOrder + nShOrder + 2)/2 + nShOrder >,3> ImageType;
  typedef itk::ShToRgbImageFilter<ImageType, nShOrder> FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  typename ImageType::Pointer itkvol = ImageType::New();
  mitk::CastToItkImage(this, itkvol);
  filter->SetInput(itkvol);
  filter->Update();

  itk::Image<itk::RGBAPixel<unsigned char>,3>::Pointer tmp = filter->GetOutput();
  m_RgbImage = mitk::Image::New();
  m_RgbImage->InitializeByItk( tmp.GetPointer() );
  m_RgbImage->SetVolume( tmp->GetBufferPointer() );
}

void mitk::ShImage::ConstructRgbImage() const
{
  switch (this->m_ImageDescriptor->GetChannelTypeById(0).GetNumberOfComponents())
  {
  case 6:
    Construct<2>();
    break;
  case 15:
    Construct<4>();
    break;
  case 28:
    Construct<6>();
    break;
  case 45:
    Construct<8>();
    break;
  case 66:
    Construct<10>();
    break;
  case 91:
    Construct<12>();
    break;
  default :
    mitkThrow() << "SH order larger 12 not supported";
  }
}

const vtkImageData* mitk::ShImage::GetNonRgbVtkImageData(int t, int n) const
{
  return Superclass::GetVtkImageData(t,n);
}

vtkImageData* mitk::ShImage::GetNonRgbVtkImageData(int t, int n)
{
  return Superclass::GetVtkImageData(t,n);
}

void mitk::ShImage::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  os << indent << "Spherical harmonics order: " << m_ShOrder << std::endl;
  os << indent << "Number of coefficients: " << m_NumCoefficients << std::endl;
  Superclass::PrintSelf(os, indent);
}
