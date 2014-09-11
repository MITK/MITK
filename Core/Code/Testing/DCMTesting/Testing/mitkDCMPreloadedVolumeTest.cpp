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

bool CheckAllPropertiesAreInOtherList(const mitk::PropertyList* list, const mitk::PropertyList* otherList)
{
  MITK_TEST_CONDITION_REQUIRED(list && otherList, "Comparison is passed two non-empty property lists")

  const mitk::PropertyList::PropertyMap* listM = list->GetMap();
  const mitk::PropertyList::PropertyMap* otherListM = otherList->GetMap();

  bool equal = true;
  for ( mitk::PropertyList::PropertyMap::const_iterator iter = listM->begin();
        iter != listM->end();
        ++iter )
  {
    std::string key = iter->first;
    mitk::BaseProperty* property = iter->second;

    mitk::PropertyList::PropertyMap::const_iterator otherEntry = otherListM->find( key );
    MITK_TEST_CONDITION( otherEntry != otherListM->end(), "  Property '" << key << "' is contained in other list" )

    mitk::BaseProperty* otherProperty = otherEntry->second;
    MITK_TEST_CONDITION( equal &= (*property == *otherProperty), "  Property '" << key << "' is equal in both list" )
  }

  return equal;
}

bool VerifyPropertyListsEquality(const mitk::PropertyList* testList, const mitk::PropertyList* referenceList)
{
  bool allTestPropsInReference = CheckAllPropertiesAreInOtherList(testList, referenceList);
  MITK_TEST_CONDITION(allTestPropsInReference, "All test properties found in reference properties")
  bool allReferencePropsInTest = CheckAllPropertiesAreInOtherList(referenceList, testList);
  MITK_TEST_CONDITION(allReferencePropsInTest, "All reference properties found in test properties")
  return allTestPropsInReference && allReferencePropsInTest;
}

int mitkDCMPreloadedVolumeTest(int argc, char** const argv)
{
  MITK_TEST_BEGIN("DCMPreloadedVolume")

  mitk::TestDCMLoading loader;
  mitk::TestDCMLoading::StringContainer files;

  // adapt expectations depending on configuration
  std::string configuration = mitk::DicomSeriesReader::GetConfigurationString();
  MITK_TEST_OUTPUT(<< "Configuration: " << configuration)

  // load files from commandline
  for (int arg = 1; arg < argc; ++arg)
  {
    MITK_TEST_OUTPUT(<< "Test file " << argv[arg])
    files.push_back( argv[arg] );
  }


  // verify all files are DCM
  for (mitk::TestDCMLoading::StringContainer::const_iterator fileIter = files.begin();
       fileIter != files.end();
       ++fileIter)
  {
    MITK_TEST_CONDITION_REQUIRED( mitk::DicomSeriesReader::IsDicom(*fileIter) , *fileIter << " is recognized as loadable DCM object" )

  }

  // load for a first time
  mitk::TestDCMLoading::ImageList images = loader.LoadFiles(files);
  MITK_TEST_OUTPUT(<< "Loaded " << images.size() << " images. Remembering properties of the first one for later comparison.")
  mitk::Image::Pointer firstImage = images.front();

  mitk::PropertyList::Pointer originalProperties = firstImage->GetPropertyList(); // save the original
  firstImage->SetPropertyList( mitk::PropertyList::New() ); // clear image properties
  // what about DEFAULT properties? currently ONLY nodes get default properties

  // load for a second time, this time provide the image volume as a pointer
  // expectation is that the reader will provide the same properties to this image (without actually loading a new mitk::Image)
  mitk::TestDCMLoading::ImageList reloadedImages = loader.LoadFiles(files, firstImage);
  MITK_TEST_OUTPUT(<< "Again loaded " << reloadedImages.size() << " images. Comparing to previously loaded version.")
  mitk::Image::Pointer reloadedImage = reloadedImages.front();

  mitk::PropertyList::Pointer regeneratedProperties = reloadedImage->GetPropertyList(); // get the version of the second load attempt

  bool listsAreEqual = VerifyPropertyListsEquality(regeneratedProperties, originalProperties);
  MITK_TEST_CONDITION(listsAreEqual, "LoadDicomSeries generates a valid property list when provided a pre-loaded image");

  MITK_TEST_END()
}
