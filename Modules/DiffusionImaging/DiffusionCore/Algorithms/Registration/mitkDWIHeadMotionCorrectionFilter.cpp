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


#ifndef MITKDIFFUSIONIMAGETODIFFUSIONIMAGEFILTER_CPP
#define MITKDIFFUSIONIMAGETODIFFUSIONIMAGEFILTER_CPP

#include "mitkDWIHeadMotionCorrectionFilter.h"

#include "itkSplitDWImageFilter.h"
#include "itkB0ImageExtractionToSeparateImageFilter.h"

#include "mitkImageTimeSelector.h"

#include "mitkPyramidImageRegistrationMethod.h"

#include "mitkIOUtil.h"

template< typename DiffusionPixelType>
mitk::DWIHeadMotionCorrectionFilter<DiffusionPixelType>
::DWIHeadMotionCorrectionFilter()
{

}

template< typename DiffusionPixelType>
void mitk::DWIHeadMotionCorrectionFilter<DiffusionPixelType>
::GenerateData()
{
  typedef itk::SplitDWImageFilter< DiffusionPixelType, DiffusionPixelType> SplitFilterType;

  DiffusionImageType* input = const_cast<DiffusionImageType*>(this->GetInput(0));

  /*
   * (1) Extract the b-zero images to a 3d+t image, register them by NCorr metric and
   *     rigid registration : they will then be used are reference image for registering
   *     the gradient images
   */
  typedef itk::B0ImageExtractionToSeparateImageFilter< DiffusionPixelType, DiffusionPixelType> B0ExtractorType;
  typename B0ExtractorType::Pointer b0_extractor = B0ExtractorType::New();
  b0_extractor->SetInput( input->GetVectorImage() );
  b0_extractor->SetDirections( input->GetDirections() );
  b0_extractor->Update();

  mitk::Image::Pointer b0Image = mitk::Image::New();
  b0Image->InitializeByItk( b0_extractor->GetOutput() );
  b0Image->SetImportChannel( b0_extractor->GetOutput()->GetBufferPointer(),
                             mitk::Image::CopyMemory );

  /* (2.1) Use the extractor to access the extracted b0 volumes */
  mitk::ImageTimeSelector::Pointer t_selector =
      mitk::ImageTimeSelector::New();

  t_selector->SetInput( b0Image );
  t_selector->SetTimeNr(0);
  t_selector->Update();

  // first unweighted image as reference space for the registration
  mitk::Image::Pointer b0referenceImage = t_selector->GetOutput();

  mitk::PyramidImageRegistrationMethod::Pointer registrationMethod = mitk::PyramidImageRegistrationMethod::New();
  registrationMethod->SetFixedImage( b0referenceImage );
  registrationMethod->SetTransformToRigid();

  // the unweighted images are of same modality
  registrationMethod->SetCrossModalityOff();

  // Initialize the temporary output image
  mitk::Image::Pointer registeredB0Image = b0Image->Clone();
  const unsigned int numberOfb0Images = b0Image->GetTimeSteps();

  mitk::ImageTimeSelector::Pointer t_selector2 =
      mitk::ImageTimeSelector::New();

  t_selector2->SetInput( b0Image );

  for( unsigned int i=1; i<numberOfb0Images; i++)
  {

    t_selector2->SetTimeNr(i);
    t_selector2->Update();

    registrationMethod->SetMovingImage( t_selector2->GetOutput() );

    try
    {
      MITK_INFO << " === (" << i <<"/"<< numberOfb0Images << ") :: Starting registration";
      registrationMethod->Update();
    }
    catch( const itk::ExceptionObject& e)
    {
      mitkThrow() << "Failed to register the b0 images, the PyramidRegistration threw an exception: \n" << e.what();
    }

    // import volume to the inter-results
    registeredB0Image->SetImportVolume( registrationMethod->GetResampledMovingImage()->GetData(),
                                       i, 0, mitk::Image::ReferenceMemory );

  }

  /*
   * (2) Split the diffusion image into a 3d+t regular image
   */
  typename SplitFilterType::Pointer split_filter = SplitFilterType::New();
  split_filter->SetInput(input->GetVectorImage() );
  split_filter->SetExtractAll();

  try
  {
    split_filter->Update();
  }
  catch( const itk::ExceptionObject &e)
  {
    mitkThrow() << " Caught exception from SplitImageFilter : " << e.what();
  }

  mitk::Image::Pointer splittedImage = mitk::Image::New();
  splittedImage->InitializeByItk( split_filter->GetOutput() );
  splittedImage->SetImportChannel( split_filter->GetOutput()->GetBufferPointer(),
                                   mitk::Image::CopyMemory );


  /*
   * (3) Use again the time-selector to access the components separately in order
   *     to perform the registration of  Image -> unweighted reference
   */




}

template< typename DiffusionPixelType>
void mitk::DWIHeadMotionCorrectionFilter<DiffusionPixelType>
::GenerateOutputInformation()
{
  if( ! this->GetInput(0) )
  {
    mitkThrow() << "No input specified!";
  }
}

#endif // MITKDIFFUSIONIMAGETODIFFUSIONIMAGEFILTER_CPP
