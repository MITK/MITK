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
//#include "mitkRegistrationMethodITK4.h"
#include "mitkImageToDiffusionImageSource.h"

#include "mitkDiffusionImageCorrectionFilter.h"
#include <mitkImageWriteAccessor.h>

#include <vector>

#include "mitkIOUtil.h"
#include <itkImage.h>

template< typename DiffusionPixelType>
mitk::DWIHeadMotionCorrectionFilter<DiffusionPixelType>
  ::DWIHeadMotionCorrectionFilter()
  : m_CurrentStep(0),
    m_Steps(100),
    m_IsInValidState(true),
    m_AbortRegistration(false),
    m_AverageUnweighted(true)
{

}

template< typename DiffusionPixelType>
void mitk::DWIHeadMotionCorrectionFilter<DiffusionPixelType>
::GenerateData()
{
  typedef itk::SplitDWImageFilter< DiffusionPixelType, DiffusionPixelType> SplitFilterType;

  DiffusionImageType* input = const_cast<DiffusionImageType*>(this->GetInput(0));

  typedef mitk::PyramidImageRegistrationMethod RegistrationMethod;
  // typedef mitk::RegistrationMethodITKV4 RegistrationMethod

  unsigned int numberOfSteps = input->GetVectorImage()->GetNumberOfComponentsPerPixel () ;
  m_Steps = numberOfSteps;
  //
  //  (1) Extract the b-zero images to a 3d+t image, register them by NCorr metric and
  //     rigid registration : they will then be used are reference image for registering
  //     the gradient images
  //
  typedef itk::B0ImageExtractionToSeparateImageFilter< DiffusionPixelType, DiffusionPixelType> B0ExtractorType;
  typename B0ExtractorType::Pointer b0_extractor = B0ExtractorType::New();
  b0_extractor->SetInput( input->GetVectorImage() );
  b0_extractor->SetDirections( input->GetDirections() );
  b0_extractor->Update();

  mitk::Image::Pointer b0Image = mitk::Image::New();
  b0Image->InitializeByItk( b0_extractor->GetOutput() );
  b0Image->SetImportChannel( b0_extractor->GetOutput()->GetBufferPointer(),
                             mitk::Image::CopyMemory );

  // (2.1) Use the extractor to access the extracted b0 volumes
  mitk::ImageTimeSelector::Pointer t_selector =
      mitk::ImageTimeSelector::New();

  t_selector->SetInput( b0Image );
  t_selector->SetTimeNr(0);
  t_selector->Update();

  // first unweighted image as reference space for the registration
  mitk::Image::Pointer b0referenceImage = t_selector->GetOutput();
  const unsigned int numberOfb0Images = b0Image->GetTimeSteps();

  // register b-zeros only if the flag to average is set, use the first one otherwise
  if( m_AverageUnweighted && numberOfb0Images > 1)
  {

    RegistrationMethod::Pointer registrationMethod = RegistrationMethod::New();
    registrationMethod->SetFixedImage( b0referenceImage );
    registrationMethod->SetTransformToRigid();

    // the unweighted images are of same modality
    registrationMethod->SetCrossModalityOff();

    // use the advanced (windowed sinc) interpolation
    registrationMethod->SetUseAdvancedInterpolation(true);

    // Initialize the temporary output image
    mitk::Image::Pointer registeredB0Image = b0Image->Clone();

    mitk::ImageTimeSelector::Pointer t_selector2 =
          mitk::ImageTimeSelector::New();

    t_selector2->SetInput( b0Image );

    for( unsigned int i=1; i<numberOfb0Images; i++)
      {

        m_CurrentStep = i + 1;
        if( m_AbortRegistration == true) {
          m_IsInValidState = false;
          mitkThrow() << "Stopped by user.";
        };

        t_selector2->SetTimeNr(i);
        t_selector2->Update();

        registrationMethod->SetMovingImage( t_selector2->GetOutput() );

        try
        {
          MITK_INFO << " === (" << i <<"/"<< numberOfb0Images-1 << ") :: Starting registration";
          registrationMethod->Update();
        }
        catch( const itk::ExceptionObject& e)
        {
          m_IsInValidState = false;
          mitkThrow() << "Failed to register the b0 images, the PyramidRegistration threw an exception: \n" << e.what();
        }

      // import volume to the inter-results
      mitk::ImageWriteAccessor imac(registrationMethod->GetResampledMovingImage());
      registeredB0Image->SetImportVolume( imac.GetData(),
        i, 0, mitk::Image::ReferenceMemory );
      }

    // use the accumulateImageFilter as provided by the ItkAccumulateFilter method in the header file
    AccessFixedDimensionByItk_1(registeredB0Image, ItkAccumulateFilter, (4), b0referenceImage );

  }


  //
  // (2) Split the diffusion image into a 3d+t regular image, extract only the weighted images
  //
  typename SplitFilterType::Pointer split_filter = SplitFilterType::New();
  split_filter->SetInput (input->GetVectorImage() );
  split_filter->SetExtractAllAboveThreshold(8, input->GetBValueMap() );

  try
  {
    split_filter->Update();
  }
  catch( const itk::ExceptionObject &e)
  {
    m_IsInValidState = false;
    mitkThrow() << " Caught exception from SplitImageFilter : " << e.what();
  }

  mitk::Image::Pointer splittedImage = mitk::Image::New();
  splittedImage->InitializeByItk( split_filter->GetOutput() );
  splittedImage->SetImportChannel( split_filter->GetOutput()->GetBufferPointer(),
                                   mitk::Image::CopyMemory );


  //
  // (3) Use again the time-selector to access the components separately in order
  //     to perform the registration of  Image -> unweighted reference
  //

  RegistrationMethod::Pointer weightedRegistrationMethod
      = RegistrationMethod::New();

  weightedRegistrationMethod->SetTransformToAffine();
  weightedRegistrationMethod->SetCrossModalityOn();

  //
  //   - (3.1) Set the reference image
  //      - a single b0 image
  //      - average over the registered b0 images if multiple present
  //
  weightedRegistrationMethod->SetFixedImage( b0referenceImage );
  // use the advanced (windowed sinc) interpolation
  weightedRegistrationMethod->SetUseAdvancedInterpolation(true);
  weightedRegistrationMethod->SetVerboseOn();

  //
  //   - (3.2) Register all timesteps in the splitted image onto the first reference
  //
  unsigned int maxImageIdx = splittedImage->GetTimeSteps();
  mitk::TimeGeometry* tsg = splittedImage->GetTimeGeometry();
  mitk::ProportionalTimeGeometry* ptg = dynamic_cast<ProportionalTimeGeometry*>(tsg);
  ptg->Expand(maxImageIdx+1);
  ptg->SetTimeStepGeometry( ptg->GetGeometryForTimeStep(0), maxImageIdx );


  mitk::Image::Pointer registeredWeighted = mitk::Image::New();
  registeredWeighted->Initialize( splittedImage->GetPixelType(0), *tsg );

  // insert the first unweighted reference as the first volume
  // in own scope to release the accessor asap after copy
  {
    mitk::ImageWriteAccessor imac(b0referenceImage);
    registeredWeighted->SetImportVolume( imac.GetData(),
      0,0, mitk::Image::CopyMemory );
  }


  // mitk::Image::Pointer registeredWeighted = splittedImage->Clone();
  // this time start at 0, we have only gradient images in the 3d+t file
  // the reference image comes form an other image
  mitk::ImageTimeSelector::Pointer t_selector_w =
      mitk::ImageTimeSelector::New();

  t_selector_w->SetInput( splittedImage );

  // store the rotation parts of the transformations in a vector
  typedef RegistrationMethod::TransformMatrixType MatrixType;
  std::vector< MatrixType > estimated_transforms;


  for( unsigned int i=0; i<maxImageIdx; i++)
  {
    m_CurrentStep = numberOfb0Images + i + 1;
    if( m_AbortRegistration == true) {
      m_IsInValidState = false;
      mitkThrow() << "Stopped by user.";
    };

    t_selector_w->SetTimeNr(i);
    t_selector_w->Update();

    weightedRegistrationMethod->SetMovingImage( t_selector_w->GetOutput() );

    try
    {
      MITK_INFO << " === (" << i+1 <<"/"<< maxImageIdx << ") :: Starting registration";
      weightedRegistrationMethod->Update();
    }
    catch( const itk::ExceptionObject& e)
    {
      m_IsInValidState = false;
      mitkThrow() << "Failed to register the b0 images, the PyramidRegistration threw an exception: \n" << e.what();
    }

    // allow expansion
    mitk::ImageWriteAccessor imac(weightedRegistrationMethod->GetResampledMovingImage());
    registeredWeighted->SetImportVolume( imac.GetData(),
      i+1, 0, mitk::Image::CopyMemory);

    estimated_transforms.push_back( weightedRegistrationMethod->GetLastRotationMatrix() );
  }


  //
  // (4) Cast the resulting image back to an diffusion weighted image
  //
  typename DiffusionImageType::GradientDirectionContainerType *gradients = input->GetDirections();
  typename DiffusionImageType::GradientDirectionContainerType::Pointer gradients_new =
      DiffusionImageType::GradientDirectionContainerType::New();
  typename DiffusionImageType::GradientDirectionType bzero_vector;
  bzero_vector.fill(0);

  // compose the direction vector
  //  - no direction for the first image
  //  - correct ordering of the directions based on the index list
  gradients_new->push_back( bzero_vector );

  typename SplitFilterType::IndexListType index_list = split_filter->GetIndexList();
  typename SplitFilterType::IndexListType::const_iterator lIter = index_list.begin();

  while( lIter != index_list.end() )
  {
    gradients_new->push_back( gradients->at( *lIter ) );
    ++lIter;
  }

  typename mitk::ImageToDiffusionImageSource< DiffusionPixelType >::Pointer caster =
      mitk::ImageToDiffusionImageSource< DiffusionPixelType >::New();

  caster->SetImage( registeredWeighted );
  caster->SetBValue( input->GetReferenceBValue() );
  caster->SetGradientDirections( gradients_new.GetPointer() );

  try
  {
    caster->Update();
  }
  catch( const itk::ExceptionObject& e)
  {
    m_IsInValidState = false;
    MITK_ERROR << "Casting back to diffusion image failed: ";
    mitkThrow() << "Subprocess failed with exception: " << e.what();
  }

  //
  // (5) Adapt the gradient directions according to the estimated transforms
  //
  typedef mitk::DiffusionImageCorrectionFilter< DiffusionPixelType > CorrectionFilterType;
  typename CorrectionFilterType::Pointer corrector = CorrectionFilterType::New();

  OutputImagePointerType output = caster->GetOutput();
  corrector->SetImage( output );
  corrector->CorrectDirections( estimated_transforms );

  //
  // (6) Pass the corrected image to the filters output port
  //
  m_CurrentStep += 1;
  this->GetOutput()->SetVectorImage(output->GetVectorImage());
  this->GetOutput()->SetReferenceBValue(output->GetReferenceBValue());
  this->GetOutput()->SetMeasurementFrame(output->GetMeasurementFrame());
  this->GetOutput()->SetDirections(output->GetDirections());
  this->GetOutput()->InitializeFromVectorImage();
  this->GetOutput()->Modified();
}

#endif // MITKDIFFUSIONIMAGETODIFFUSIONIMAGEFILTER_CPP
