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

#include "mitkDICOMFileReaderSelector.h"
#include "mitkDICOMFileReaderTestHelper.h"

#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleResourceStream.h>
#include <usModule.h>
#include <usModuleRegistry.h>

#include "mitkImageToItk.h"
#include "itkImageRegionConstIteratorWithIndex.h"

#include "mitkTestingMacros.h"

/**
  \brief Verify single block reading of SimpleDICOMvolumeImport for sloppily tagged data.
*/
int mitkDICOMSimpleVolumeImportTest(int argc, char* argv[])
{
  typedef mitk::DICOMFileReaderSelector::ReaderList           ReaderList;
  typedef itk::Image< short, 3 >                              ImageType;
  typedef itk::ImageRegionConstIteratorWithIndex< ImageType > IteratorType;

  MITK_TEST_BEGIN("mitkDICOMSimpleVolumeImportTest");

  us::ModuleResource resource =
    us::ModuleRegistry::GetModule("MitkDICOMReader")->GetResource("configurations/3D/simpleinstancenumber_soft.xml");

  std::string descr;
  if ( resource.IsValid() )
  {
    us::ModuleResourceStream stream(resource);

    stream.seekg(0, std::ios::end);
    descr.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);

    descr.assign((std::istreambuf_iterator<char>(stream)),
    std::istreambuf_iterator<char>());
  }
  mitk::DICOMFileReaderSelector::Pointer readerSelector = mitk::DICOMFileReaderSelector::New();

  readerSelector->AddConfig( descr );
  ReaderList readers = readerSelector->GetAllConfiguredReaders();

  mitk::DICOMFileReader::Pointer simpleReader;
  for ( ReaderList::const_iterator it = readers.begin(); it != readers.end(); ++it )
  {
    mitk::DICOMFileReader::Pointer reader = *it;
    std::string configLabel = reader->GetConfigurationLabel();
    if ( configLabel == "Instance Number, non-consecutive, simple import" )
    {
      simpleReader = reader;
    }
  }

  MITK_TEST_CONDITION_REQUIRED( simpleReader.IsNotNull(), "DICOMFileReaderSelector is able to create simple reader from XML");

  if ( argc > 1 )
  {
    mitk::DICOMFileReaderTestHelper::SetTestInputFilenames( argc,argv );

    mitk::DICOMFileReaderTestHelper::TestInputFilenames( simpleReader );
    mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( simpleReader );
    mitk::DICOMFileReaderTestHelper::TestSingleBlockIsRead( simpleReader );

    const mitk::DICOMImageBlockDescriptor block = simpleReader->GetOutput( 0 );
    const mitk::Image::Pointer mitkImage = block.GetMitkImage();

    ImageType::Pointer itkImage = mitk::ImageToItkImage<short,3>( mitkImage );

    IteratorType it( itkImage, itkImage->GetLargestPossibleRegion() );
    bool success = true;
    while ( !it.IsAtEnd() )
    {
      ImageType::PixelType val = it.Get();
      ImageType::IndexType ind = it.GetIndex();

      if ( !(val == ind[2]) )
      {
        success = false;
        break;
      }

      ++it;
    }

    MITK_TEST_CONDITION( success, "Single block image composed as expected.");
  }

  MITK_TEST_END();
}

