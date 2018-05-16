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

#include "mitkTestingMacros.h"
#include "mitkIOUtil.h"

#include <itkElectrostaticRepulsionDiffusionGradientReductionFilter.h>

#include "mitkDWIHeadMotionCorrectionFilter.h"
#include <mitkDiffusionPropertyHelper.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>

typedef short                                       DiffusionPixelType;


int mitkExtractSingleShellTest( int argc, char* argv[] )
{
  MITK_TEST_BEGIN("mitkExtractSingleShellTest");

  MITK_TEST_CONDITION_REQUIRED( argc > 3, "Specify input and output and the shell to be extracted");

  /*
    1. Get input data
    */
  mitk::Image::Pointer dwimage = mitk::IOUtil::Load<mitk::Image>( argv[1] );

  mitk::GradientDirectionsProperty::Pointer gradientsProperty = static_cast<mitk::GradientDirectionsProperty *>( dwimage->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() );

  MITK_TEST_CONDITION_REQUIRED( gradientsProperty.IsNotNull(), "Input is a dw-image");

  unsigned int extract_value = 0;
  std::istringstream input(argv[3]);

  input >> extract_value;

  typedef itk::ElectrostaticRepulsionDiffusionGradientReductionFilter<DiffusionPixelType, DiffusionPixelType> FilterType;
  typedef mitk::DiffusionPropertyHelper::BValueMapType BValueMap;

  // GetShellSelection from GUI
  BValueMap shellSelectionMap;
  BValueMap originalShellMap = static_cast<mitk::BValueMapProperty*>(dwimage->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();
  std::vector<unsigned int> newNumGradientDirections;

  shellSelectionMap[extract_value] = originalShellMap[extract_value];
  newNumGradientDirections.push_back( originalShellMap[extract_value].size() ) ;

  itk::VectorImage< short, 3 >::Pointer itkVectorImagePointer = itk::VectorImage< short, 3 >::New();
  mitk::CastToItkImage(dwimage, itkVectorImagePointer);
  itk::VectorImage< short, 3 > *vectorImage = itkVectorImagePointer.GetPointer();

  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientContainer = static_cast<mitk::GradientDirectionsProperty*>( dwimage->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(vectorImage);
  filter->SetOriginalGradientDirections(gradientContainer);
  filter->SetNumGradientDirections(newNumGradientDirections);
  filter->SetOriginalBValueMap(originalShellMap);
  filter->SetShellSelectionBValueMap(shellSelectionMap);

  try
  {
    filter->Update();
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_TEST_FAILED_MSG( << "Failed due to ITK exception: " << e.what() );
  }

  mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( filter->GetOutput() );
  mitk::DiffusionPropertyHelper::CopyProperties(dwimage, outImage, true);
  outImage->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetGradientDirections() ) );
  mitk::DiffusionPropertyHelper propertyHelper( outImage );
  propertyHelper.InitializeImage();

  /*
   * 3. Write output data
   **/
  try
  {
    mitk::IOUtil::Save(outImage, argv[2]);
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Catched exception: " << e.what();
    mitkThrow() << "Failed with exception from subprocess!";
  }

  MITK_TEST_END();
}
