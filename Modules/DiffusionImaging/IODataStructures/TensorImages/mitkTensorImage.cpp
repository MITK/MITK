/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2008-02-08 11:19:03 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
    ConstructRgbImage();
  return m_RgbImage->GetVtkImageData(t,n);
}

void mitk::TensorImage::ConstructRgbImage()
{
  typedef itk::Image<itk::DiffusionTensor3D<float>,3> ImageType;
  typedef itk::TensorToRgbImageFilter<ImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();
  
  ImageType::Pointer itkvol = ImageType::New();
  mitk::CastToItkImage<ImageType>(this, itkvol);
  filter->SetInput(itkvol);
  filter->Update();

  m_RgbImage = mitk::Image::New();
  m_RgbImage->InitializeByItk( filter->GetOutput() );
  m_RgbImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
}

vtkImageData* mitk::TensorImage::GetNonRgbVtkImageData(int t, int n)
{
  return Superclass::GetVtkImageData(t,n);
}

//
//vtkImageData* mitk::TensorImage::GetRgbVtkImageData(int t, int n)
//{
//  if(m_Initialized==false)
//  {
//    if(GetSource()==NULL)
//      return NULL;
//    if(GetSource()->Updating()==false)
//      GetSource()->UpdateOutputInformation();
//  }
//
//  if(m_VtkImageData==NULL)
//    ConstructVtkImageData();
//  return m_VtkImageData;
//
//  ImageDataItemPointer volume=GetVolumeData(t, n);
//  if(volume.GetPointer()==NULL || volume->GetVtkImageData() == NULL)
//    return NULL;
//
//#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
//  float *fspacing = const_cast<float *>(GetSlicedGeometry(t)->GetFloatSpacing());
//  double dspacing[3] = {fspacing[0],fspacing[1],fspacing[2]};
//  volume->GetVtkImageData()->SetSpacing( dspacing );
//#else
//  volume->GetVtkImageData()->SetSpacing(const_cast<float*>(GetSlicedGeometry(t)->GetFloatSpacing()));
//#endif
//  return volume->GetVtkImageData();
//}
//
//void mitk::TensorImage::ConstructRgbVtkImageData(int t, int n) const
//{
//  vtkImageData *inData = vtkImageData::New();
//  vtkDataArray *scalars = NULL;
//
//  mitkIpPicDescriptor* picDescriptor = GetVolumeData(t,n)->GetPicDescriptor();
//
//  unsigned long size = 0;
//  if ( picDescriptor->dim == 1 )
//  {
//    inData->SetDimensions( picDescriptor->n[0] -1, 1, 1);
//    size = picDescriptor->n[0];
//    inData->SetOrigin( ((float) picDescriptor->n[0]) / 2.0f, 0, 0 );
//  }
//  else if ( picDescriptor->dim == 2 )
//  {
//    inData->SetDimensions( picDescriptor->n[0] , picDescriptor->n[1] , 1 );
//    size = picDescriptor->n[0] * picDescriptor->n[1];
//    inData->SetOrigin( ((float) picDescriptor->n[0]) / 2.0f, ((float) picDescriptor->n[1]) / 2.0f, 0 );
//  }
//  else if ( picDescriptor->dim >= 3 )
//  {
//    inData->SetDimensions( picDescriptor->n[0], picDescriptor->n[1], picDescriptor->n[2] );
//    size = picDescriptor->n[0] * picDescriptor->n[1] * picDescriptor->n[2];
//    // Test
//    //inData->SetOrigin( (float) picDescriptor->n[0] / 2.0f, (float) picDescriptor->n[1] / 2.0f, (float) picDescriptor->n[2] / 2.0f );
//    inData->SetOrigin( 0, 0, 0 );
//  }
//  else
//  {
//    inData->Delete () ;
//    return;
//  }
//
//  // allocate new scalars with three components for RGB
//  inData->SetNumberOfScalarComponents(3);
//  inData->SetScalarType( VTK_UNSIGNED_CHAR );
//  scalars = vtkUnsignedCharArray::New();
//  m_VtkImageDataTensor = inData;
//  scalars->SetNumberOfComponents(m_VtkImageDataTensor->GetNumberOfScalarComponents());
//
//  // calculate RGB information from tensors
//  if(m_PixelType == typeid(itk::DiffusionTensor3D<float>))
//    scalars->SetVoidArray(ConvertTensorsToRGB<itk::DiffusionTensor3D<float> >(), m_Size/2, 1);
//  if(m_PixelType == typeid(itk::DiffusionTensor3D<double>))
//    scalars->SetVoidArray(ConvertTensorsToRGB<itk::DiffusionTensor3D<double> >(), m_Size/2, 1);
//
//  m_VtkImageDataTensor->GetPointData()->SetScalars(scalars);
//  scalars->Delete();
//
//}
//
///**
//* This method calculates RGB image from tensor information.
//* Templated over pixeltype, output always three uchar components.
//*/
//template <class TPixeltype>
//unsigned char *mitk::ImageDataItem::ConvertTensorsToRGB() const
//{
//  const unsigned char *p   = m_Data;
//  unsigned char *out = (unsigned char *) malloc(m_Size/2);
//  const int pixelsize = sizeof(TPixeltype);
//  const int numIts = m_Size/pixelsize;
//
//#ifdef _OPENMP
//#pragma omp parallel for
//#endif
//  for(int i=0; i<numIts; i++)
//  {
//    const TPixeltype* tensor = static_cast<TPixeltype* >((void*)(p+i*pixelsize));
//    typename TPixeltype::EigenValuesArrayType eigenvalues;
//    typename TPixeltype::EigenVectorsMatrixType eigenvectors;
//    tensor->ComputeEigenAnalysis(eigenvalues, eigenvectors);
//
//    int index = 2;
//    if( (eigenvalues[0] >= eigenvalues[1])
//      && (eigenvalues[0] >= eigenvalues[2]) )
//      index = 0;
//    else if(eigenvalues[1] >= eigenvalues[2])
//      index = 1;
//
//    const float fa = tensor->GetFractionalAnisotropy();
//    float r = abs(eigenvectors(index,0)) * fa;
//    float g = abs(eigenvectors(index,1)) * fa;
//    float b = abs(eigenvectors(index,2)) * fa;
//
//    __IMG_DAT_ITEM__CEIL_ZERO_ONE__(r);
//    __IMG_DAT_ITEM__CEIL_ZERO_ONE__(g);
//    __IMG_DAT_ITEM__CEIL_ZERO_ONE__(b);
//
//    *(out+i*3+0) = (unsigned char)( 255.0f * r );
//    *(out+i*3+1) = (unsigned char)( 255.0f * g );
//    *(out+i*3+2) = (unsigned char)( 255.0f * b );
//  }
//  return out;
//}