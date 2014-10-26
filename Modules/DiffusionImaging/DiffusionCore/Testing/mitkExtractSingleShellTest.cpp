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

typedef short                                       DiffusionPixelType;
typedef mitk::DiffusionImage< DiffusionPixelType >  DiffusionImageType;


int mitkExtractSingleShellTest( int argc, char* argv[] )
{
  MITK_TEST_BEGIN("mitkExtractSingleShellTest");

  MITK_TEST_CONDITION_REQUIRED( argc > 3, "Specify input and output and the shell to be extracted");

  /*
    1. Get input data
    */
  mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage( argv[1] );
  DiffusionImageType* dwimage =
      static_cast<DiffusionImageType*>( inputImage.GetPointer() );

  MITK_TEST_CONDITION_REQUIRED( dwimage != NULL, "Input is a dw-image");

  unsigned int extract_value = 0;
  std::istringstream input(argv[3]);

  input >> extract_value;

  typedef itk::ElectrostaticRepulsionDiffusionGradientReductionFilter<DiffusionPixelType, DiffusionPixelType> FilterType;
  typedef DiffusionImageType::BValueMap BValueMap;

  // GetShellSelection from GUI
  BValueMap shellSelectionMap;
  BValueMap originalShellMap = dwimage->GetBValueMap();
  std::vector<unsigned int> newNumGradientDirections;

  shellSelectionMap[extract_value] = originalShellMap[extract_value];
  newNumGradientDirections.push_back( originalShellMap[extract_value].size() ) ;

  DiffusionImageType::GradientDirectionContainerType::Pointer gradientContainer = dwimage->GetDirections();
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(dwimage->GetVectorImage());
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

  DiffusionImageType::Pointer image = DiffusionImageType::New();
  image->SetVectorImage( filter->GetOutput() );
  image->SetReferenceBValue(dwimage->GetReferenceBValue());
  image->SetDirections(filter->GetGradientDirections());
  image->SetMeasurementFrame(dwimage->GetMeasurementFrame());
  image->InitializeFromVectorImage();
  /*
   * 3. Write output data
   **/
  try
  {
    mitk::IOUtil::Save(image, argv[2]);
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Catched exception: " << e.what();
    mitkThrow() << "Failed with exception from subprocess!";
  }

  MITK_TEST_END();
}
