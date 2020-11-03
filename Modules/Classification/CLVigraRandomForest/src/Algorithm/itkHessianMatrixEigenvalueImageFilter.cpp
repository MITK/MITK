/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ITKHESSIANMATRIXEIGENVALUEIMAGEFILTER_CPP
#define ITKHESSIANMATRIXEIGENVALUEIMAGEFILTER_CPP

#include <itkHessianMatrixEigenvalueImageFilter.h>
#include <itkImageRegionIterator.h>
#include <vigra/tensorutilities.hxx>
#include <vigra/convolution.hxx>
#include <mitkCLUtil.h>


template< class TInputImageType, class TOutputImageType, class TMaskImageType>
void itk::HessianMatrixEigenvalueImageFilter<TInputImageType,TOutputImageType, TMaskImageType>::GenerateOutputInformation()
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
void itk::HessianMatrixEigenvalueImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::GenerateData()
{

  typedef typename TInputImageType::PixelType InputPixelType;

  typename TInputImageType::RegionType region = this->GetInput()->GetLargestPossibleRegion();
  unsigned int xdim = region.GetSize(0);
  unsigned int ydim = region.GetSize(1);
  unsigned int zdim = region.GetSize(2);

  typename TInputImageType::Pointer maske_input = TInputImageType::New();
  maske_input->SetDirection(this->GetInput()->GetDirection());
  maske_input->SetSpacing(this->GetInput()->GetSpacing());
  maske_input->SetRegions(this->GetInput()->GetLargestPossibleRegion());
  maske_input->Allocate();

  itk::ImageRegionConstIterator<TInputImageType> iit(this->GetInput(), this->GetInput()->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TInputImageType> miit(maske_input, maske_input->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<TMaskImageType> mit(m_ImageMask, m_ImageMask->GetLargestPossibleRegion());

  while(!miit.IsAtEnd())
  {
    if(mit.Value()!=0)
      miit.Set(iit.Value());
    else
      miit.Set(0);

    ++miit;
    ++mit;
    ++iit;
  }


  vigra::Shape3 shape(xdim,ydim,zdim);
  vigra::MultiArrayView<3, InputPixelType, vigra::StridedArrayTag > input_image_view(
        shape ,
        maske_input->GetBufferPointer());


  vigra::MultiArray<3, vigra::TinyVector<InputPixelType, 3> > hessian_image(shape);
  vigra::MultiArray<3, vigra::TinyVector<InputPixelType, 3> > eigenvalues_image(shape);

  for(unsigned int i = 0 ; i < zdim; ++i )
  {
    vigra::Shape2 slice_shape(xdim,ydim);
    vigra::MultiArrayView<2, InputPixelType, vigra::StridedArrayTag > image_slice(
          slice_shape,
          input_image_view.data()+ (i*xdim*ydim));

    vigra::MultiArrayView<2, vigra::TinyVector<InputPixelType, 3> > hessian_slice(
          slice_shape,
          hessian_image.data() + (i*xdim*ydim));

    //    vigra::hessianMatrixOfGaussian(input_image_slice_view, hessian_image_slice_view, sigma);

    //    vigra::hessianMatrixOfGaussian(image_slice,hessian_slice,sigma);
    vigra::hessianMatrixOfGaussian(image_slice,
                                   hessian_slice.bindElementChannel(0),
                                   hessian_slice.bindElementChannel(1),
                                   hessian_slice.bindElementChannel(2),
                                   m_Sigma);

    vigra::MultiArrayView<2, vigra::TinyVector<InputPixelType, 3> > eigenvalues_image_slice_view(
          slice_shape,
          eigenvalues_image.data() + (i*xdim*ydim));

    vigra::tensorEigenRepresentation(hessian_slice, eigenvalues_image_slice_view);
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
void itk::HessianMatrixEigenvalueImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::SetImageMask(TMaskImageType * maskimage)
{
  this->m_ImageMask = maskimage;
}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
itk::HessianMatrixEigenvalueImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::HessianMatrixEigenvalueImageFilter()
{
  this->SetNumberOfIndexedOutputs(3);
  this->SetNumberOfIndexedInputs(1);

  this->SetNthOutput( 0, this->MakeOutput(0) );
  this->SetNthOutput( 1, this->MakeOutput(1) );
  this->SetNthOutput( 2, this->MakeOutput(2) );

}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
itk::HessianMatrixEigenvalueImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::~HessianMatrixEigenvalueImageFilter()
{

}



#endif
