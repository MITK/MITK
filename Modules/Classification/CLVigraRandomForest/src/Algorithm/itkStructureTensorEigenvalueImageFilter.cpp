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

#ifndef ITKSTRUCTURETENSOREIGENVALUEIMAGEFILTER_CPP
#define ITKSTRUCTURETENSOREIGENVALUEIMAGEFILTER_CPP

#include <itkStructureTensorEigenvalueImageFilter.h>
#include <itkImageRegionIterator.h>
#include <vigra/tensorutilities.hxx>
#include <vigra/convolution.hxx>


template< class TInputImageType, class TOutputImageType, class TMaskImageType>
void itk::StructureTensorEigenvalueImageFilter<TInputImageType,TOutputImageType, TMaskImageType>::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
  this->GetOutput(0)->SetDirection(this->GetInput()->GetDirection());
  this->GetOutput(0)->SetSpacing(this->GetInput()->GetSpacing());
  this->GetOutput(0)->SetRegions(this->GetInput()->GetLargestPossibleRegion());
  this->GetOutput(0)->Allocate();

  this->GetOutput(1)->SetDirection(this->GetInput()->GetDirection());
  this->GetOutput(1)->SetSpacing(this->GetInput()->GetSpacing());
  this->GetOutput(1)->SetRegions(this->GetInput()->GetLargestPossibleRegion());
  this->GetOutput(1)->Allocate();

  this->GetOutput(2)->SetDirection(this->GetInput()->GetDirection());
  this->GetOutput(2)->SetSpacing(this->GetInput()->GetSpacing());
  this->GetOutput(2)->SetRegions(this->GetInput()->GetLargestPossibleRegion());
  this->GetOutput(2)->Allocate();
}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
void itk::StructureTensorEigenvalueImageFilter<TInputImageType,TOutputImageType, TMaskImageType>::GenerateData()
{



  typedef typename TInputImageType::PixelType InputPixelType;

  typename TInputImageType::RegionType region = this->GetInput()->GetLargestPossibleRegion();
  unsigned int xdim = region.GetSize(0);
  unsigned int ydim = region.GetSize(1);
  unsigned int zdim = region.GetSize(2);



  vigra::Shape3 shape(xdim,ydim,zdim);
  vigra::MultiArrayView<3, InputPixelType, vigra::StridedArrayTag > input_image_view(
        shape ,
        this->GetInput()->GetBufferPointer());

  vigra::MultiArray<3, vigra::TinyVector<InputPixelType, 3> > structuretensor_image(shape);
  vigra::MultiArray<3, vigra::TinyVector<InputPixelType, 3> > eigenvalues_image(shape);


  for(int i = 0 ; i < zdim; ++i )
  {
    vigra::Shape2 slice_shape(xdim,ydim);
    vigra::MultiArrayView<2, InputPixelType, vigra::StridedArrayTag > input_image_slice_view(
          slice_shape,
          input_image_view.data()+ (i*xdim*ydim));

    vigra::MultiArrayView<2, vigra::TinyVector<InputPixelType, 3> > structuretensor_image_slice_view(
          slice_shape,
          structuretensor_image.data() + (i*xdim*ydim));

    vigra::structureTensor(input_image_slice_view, structuretensor_image_slice_view, m_InnerScale, m_OuterScale);

    vigra::MultiArrayView<2, vigra::TinyVector<InputPixelType, 3> > eigenvalues_image_slice_view(
          slice_shape,
          eigenvalues_image.data() + (i*xdim*ydim));
    vigra::tensorEigenRepresentation(structuretensor_image_slice_view, eigenvalues_image_slice_view);
  }

  vigra::MultiArrayView<3, InputPixelType, vigra::StridedArrayTag > ev1_image = eigenvalues_image.bindElementChannel(0);
  vigra::MultiArrayView<3, InputPixelType, vigra::StridedArrayTag > ev2_image = eigenvalues_image.bindElementChannel(1);
  vigra::MultiArrayView<3, InputPixelType, vigra::StridedArrayTag > ev3_image = eigenvalues_image.bindElementChannel(2);

  for(unsigned int x = 0 ; x < xdim; ++x)
    for(unsigned int y = 0 ; y < ydim; ++y)
      for(unsigned int z = 0 ; z < zdim; ++z)
      {
        itk::Index<3> indx = {{x,y,z}};
        this->GetOutput(0)->operator [](indx) = ev1_image(x,y,z);
        this->GetOutput(1)->operator [](indx) = ev2_image(x,y,z);
        this->GetOutput(2)->operator [](indx) = ev3_image(x,y,z);
      }


}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
void itk::StructureTensorEigenvalueImageFilter<TInputImageType,TOutputImageType, TMaskImageType>::SetImageMask(TMaskImageType *maskimage)
{
  this->m_ImageMask = maskimage;
}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
itk::StructureTensorEigenvalueImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::StructureTensorEigenvalueImageFilter()
{
  this->SetNumberOfIndexedOutputs(3);
  this->SetNumberOfIndexedInputs(1);

  this->SetNthOutput( 0, this->MakeOutput(0) );
  this->SetNthOutput( 1, this->MakeOutput(1) );
  this->SetNthOutput( 2, this->MakeOutput(2) );

}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
itk::StructureTensorEigenvalueImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::~StructureTensorEigenvalueImageFilter()
{

}



#endif
