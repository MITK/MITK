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


#ifndef MITKDWIHEADMOTIONCORRECTIONFILTER_CPP
#define MITKDWIHEADMOTIONCORRECTIONFILTER_CPP

#include "mitkDWIHeadMotionCorrectionFilter.h"

#include "itkSplitDWImageFilter.h"
#include "itkB0ImageExtractionToSeparateImageFilter.h"

#include "mitkImageTimeSelector.h"

#include "mitkPyramidImageRegistrationMethod.h"
//#include "mitkRegistrationMethodITK4.h"
#include <mitkDiffusionPropertyHelper.h>

#include "mitkDiffusionImageCorrectionFilter.h"
#include <mitkProperties.h>

#include <vector>

#include "mitkIOUtil.h"
#include <itkImage.h>

mitk::DWIHeadMotionCorrectionFilter::DWIHeadMotionCorrectionFilter()
  : m_CurrentStep(0),
    m_Steps(100),
    m_IsInValidState(true),
    m_AbortRegistration(false),
    m_AverageUnweighted(true)
{

}


void mitk::DWIHeadMotionCorrectionFilter::GenerateData()
{
  typedef itk::SplitDWImageFilter< DiffusionPixelType, DiffusionPixelType> SplitFilterType;

  InputImageType* input = const_cast<InputImageType*>(this->GetInput(0));

  ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
  mitk::CastToItkImage(input, itkVectorImagePointer);

  typedef mitk::PyramidImageRegistrationMethod RegistrationMethod;
  // typedef mitk::RegistrationMethodITKV4 RegistrationMethod
  unsigned int numberOfSteps = itkVectorImagePointer->GetNumberOfComponentsPerPixel () ;
  m_Steps = numberOfSteps;
  //
  //  (1) Extract the b-zero images to a 3d+t image, register them by NCorr metric and
  //     rigid registration : they will then be used are reference image for registering
  //     the gradient images
  //
  typedef itk::B0ImageExtractionToSeparateImageFilter< DiffusionPixelType, DiffusionPixelType> B0ExtractorType;
  B0ExtractorType::Pointer b0_extractor = B0ExtractorType::New();
  b0_extractor->SetInput( itkVectorImagePointer );
  b0_extractor->SetDirections( static_cast<mitk::GradientDirectionsProperty*>( input->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
  b0_extractor->Update();

  mitk::Image::Pointer b0Image = mitk::Image::New();
  b0Image->InitializeByItk( b0_extractor->GetOutput() );
  b0Image->SetImportVolume( b0_extractor->GetOutput()->GetBufferPointer(),
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
      mitk::ImageRegionAccessor accessor(registrationMethod->GetResampledMovingImage());
      mitk::ImageAccessLock lock(&accessor, true);
      registeredB0Image->SetImportVolume(accessor.getData(), i, 0, mitk::Image::ReferenceMemory);
      }

    // use the accumulateImageFilter as provided by the ItkAccumulateFilter method in the header file
    AccessFixedDimensionByItk_1(registeredB0Image, ItkAccumulateFilter, (4), b0referenceImage );

  }


  //
  // (2) Split the diffusion image into a 3d+t regular image, extract only the weighted images
  //
  SplitFilterType::Pointer split_filter = SplitFilterType::New();
  split_filter->SetInput (itkVectorImagePointer );
  split_filter->SetExtractAllAboveThreshold(8, static_cast<mitk::BValueMapProperty*>(input->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap() );

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
  splittedImage->SetImportVolume( split_filter->GetOutput()->GetBufferPointer(),
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
    mitk::ImageRegionAccessor accessor(b0referenceImage);
    mitk::ImageAccessLock lock(&accessor);
    registeredWeighted->SetImportVolume(accessor.getData(),0,0, mitk::Image::CopyMemory);
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
    mitk::ImageRegionAccessor accessor(weightedRegistrationMethod->GetResampledMovingImage());
    mitk::ImageAccessLock lock(&accessor);
    registeredWeighted->SetImportVolume(accessor.getData(),i+1, 0, mitk::Image::CopyMemory);

    estimated_transforms.push_back( weightedRegistrationMethod->GetLastRotationMatrix() );
  }


  //
  // (4) Cast the resulting image back to an diffusion weighted image
  //
  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType *gradients = static_cast<mitk::GradientDirectionsProperty*>( input->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();
  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradients_new =
    mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::New();
  mitk::DiffusionPropertyHelper::GradientDirectionType bzero_vector;
  bzero_vector.fill(0);

  // compose the direction vector
  //  - no direction for the first image
  //  - correct ordering of the directions based on the index list
  gradients_new->push_back( bzero_vector );

  SplitFilterType::IndexListType index_list = split_filter->GetIndexList();
  SplitFilterType::IndexListType::const_iterator lIter = index_list.begin();

  while( lIter != index_list.end() )
  {
    gradients_new->push_back( gradients->at( *lIter ) );
    ++lIter;
  }

  registeredWeighted->SetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( gradients_new.GetPointer() ));
  registeredWeighted->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(input->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );

  //
  // (5) Adapt the gradient directions according to the estimated transforms
  //
  typedef mitk::DiffusionImageCorrectionFilter CorrectionFilterType;
  CorrectionFilterType::Pointer corrector = CorrectionFilterType::New();

  mitk::Image::Pointer output = registeredWeighted;
  corrector->SetImage( output );
  corrector->CorrectDirections( estimated_transforms );

  //
  // (6) Pass the corrected image to the filters output port
  //
  m_CurrentStep += 1;

  this->GetOutput()->Initialize( output );

  this->GetOutput()->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( static_cast<mitk::GradientDirectionsProperty*>( output->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() ) );
  this->GetOutput()->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>( output->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
  this->GetOutput()->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(output->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );

  mitk::DiffusionPropertyHelper propertyHelper( this->GetOutput() );
  propertyHelper.InitializeImage();
  this->GetOutput()->Modified();
}

#endif // MITKDWIHEADMOTIONCORRECTIONFILTER_CPP
