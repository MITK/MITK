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


#include "mitkTestDCMLoading.h"
#include "mitkTestingMacros.h"
#include "mitkImage.h"

int mitkDCMTestingSanityTest(int argc, char** const argv)
{
  MITK_TEST_BEGIN("DCMTestingSanity")

  mitk::TestDCMLoading loader;
  mitk::TestDCMLoading::StringContainer files;

  // adapt expectations depending on configuration
  std::string configuration = mitk::DicomSeriesReader::GetConfigurationString();
  MITK_TEST_OUTPUT(<< "Configuration: " << configuration)
  /*
  MITK_TEST_CONDITION_REQUIRED( configuration.find( "GDCM_VERSION: 2." ) != std::string::npos,
                                "Expect at least GDCM version 2" )
  */

  // load files from commandline
  unsigned int numberOfExpectedImages = 0;
  if (argc > 1) numberOfExpectedImages = atoi(argv[1]);
  for (int arg = 2; arg < argc; ++arg) files.push_back( argv[arg] );

  // verify all files are DCM
  for (mitk::TestDCMLoading::StringContainer::const_iterator fileIter = files.begin();
       fileIter != files.end();
       ++fileIter)
  {
    MITK_TEST_CONDITION_REQUIRED( mitk::DicomSeriesReader::IsDicom(*fileIter) , *fileIter << " is recognized as loadable DCM object" )

  }

  // compare with expected number of images from commandline
  mitk::TestDCMLoading::ImageList images = loader.LoadFiles(files);
  MITK_TEST_CONDITION_REQUIRED( images.size() == numberOfExpectedImages, "Loading " << files.size()
                                                                      << " files from commandline results in " << numberOfExpectedImages
                                                                      << " images (see test invocation)" )

  // check dump equality (dumping image information must always equal itself)
  for ( mitk::TestDCMLoading::ImageList::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter )
  {
    const mitk::Image* image = *imageIter;
    MITK_TEST_CONDITION( loader.CompareImageInformationDumps( loader.DumpImageInformation(image),
                                                              loader.DumpImageInformation(image) ) == true,
                         "Image information dumping is able to reproduce its result." )
  }

  MITK_TEST_END()
}
