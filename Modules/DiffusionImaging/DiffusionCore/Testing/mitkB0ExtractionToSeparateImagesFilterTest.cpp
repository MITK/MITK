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
#include "itkVectorImage.h"
#include "mitkBaseData.h"
#include "mitkBaseDataIOFactory.h"

#include "mitkImageWriter.h"
#include "itkImageFileWriter.h"

#include "itkB0ImageExtractionToSeparateImageFilter.h"
#include "mitkDiffusionImage.h"

#include "mitkDiffusionCoreObjectFactory.h"

/** Documentation
 *  Test for factory registration
 */
int mitkB0ExtractionToSeparateImagesFilterTest(int  argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkB0ExtractionToSeparateImagesFilterTest");

  MITK_TEST_CONDITION_REQUIRED(argc > 2, "Test image is specified. ");

  typedef short DiffusionPixelType;
  typedef mitk::DiffusionImage< DiffusionPixelType > DiffusionImageType;
  typedef mitk::NrrdDiffusionImageReader< DiffusionPixelType> DiffusionNrrdReaderType;

  RegisterDiffusionImagingObjectFactory();

  std::string inputFileName( argv[1] );

  std::vector<mitk::BaseData::Pointer> inputBaseDataVector = mitk::BaseDataIO::LoadBaseDataFromFile( inputFileName, "","",false);

  MITK_TEST_CONDITION_REQUIRED( inputBaseDataVector.size() > 0, "BaseDataIO returned non-empty vector.");

  mitk::BaseData::Pointer baseData = inputBaseDataVector.at(0);

  MITK_TEST_CONDITION_REQUIRED( baseData.IsNotNull(), "BaseData is not null")

      DiffusionImageType* vols = dynamic_cast< DiffusionImageType* >(baseData.GetPointer());

  MITK_TEST_CONDITION_REQUIRED( vols != NULL, "Casting basedata to diffusion image successfull." );

  // filter
  typedef itk::B0ImageExtractionToSeparateImageFilter< short, short> FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  MITK_TEST_CONDITION_REQUIRED(filter.IsNotNull(), "Filter instance created. ");

  filter->SetInput(vols->GetVectorImage());
  filter->SetDirections(vols->GetDirections());
  filter->Update();

  // output
  mitk::Image::Pointer mitkImage = mitk::Image::New();

  MITK_TEST_CONDITION_REQUIRED( mitkImage.IsNotNull(), "mitkImage not null." );

  mitkImage->InitializeByItk( filter->GetOutput() );

  MITK_TEST_CONDITION_REQUIRED( mitkImage->GetDimension()==4, "Output image is a 4D image.");

  mitkImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

  typedef itk::ImageFileWriter< FilterType::OutputImageType > itkImageWriterType;
  typename itkImageWriterType::Pointer itkWriter = itkImageWriterType::New();

  itkWriter->SetFileName( argv[2] );
  itkWriter->SetInput( filter->GetOutput() );

  try
  {
    itkWriter->Update();
  }
  catch(itk::ExceptionObject &e)
  {
    MITK_ERROR << "Catched exception from image writer. " << e.what();
  }

  /*
  // write output
  mitk::ImageWriter::Pointer writer = mitk::ImageWriter::New();
  MITK_TEST_CONDITION_REQUIRED( writer.IsNotNull(), "Writer instance created. ");

  writer->SetInput( mitkImage );
  writer->SetExtension(".nrrd");
  writer->SetFileName( "/localdata/hering/_Images/TestB0Extraction" );

  writer->Update();
*/
  // always end with this!
  MITK_TEST_END();
}
