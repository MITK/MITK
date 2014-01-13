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


#include "mitkTestDICOMLoading.h"
#include "mitkTestingMacros.h"

#include "mitkDICOMTagCache.h"

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

// !!! we expect that this tests get a list of files that load as ONE SINGLE mitk::Image!
int mitkDICOMPreloadedVolumeTest(int argc, char** const argv)
{
  MITK_TEST_BEGIN("DICOMPreloadedVolume")

  mitk::TestDICOMLoading loader;
  mitk::StringList files;

  // load files from commandline
  for (int arg = 1; arg < argc; ++arg)
  {
    MITK_TEST_OUTPUT(<< "Test file " << argv[arg])
    files.push_back( argv[arg] );
  }


  // verify all files are DICOM
  for (mitk::StringList::const_iterator fileIter = files.begin();
       fileIter != files.end();
       ++fileIter)
  {
    MITK_TEST_CONDITION_REQUIRED( mitk::DICOMFileReader::IsDICOM(*fileIter) , *fileIter << " is recognized as loadable DICOM object" )
  }

  // load for a first time
  mitk::TestDICOMLoading::ImageList images = loader.LoadFiles(files);
  MITK_TEST_OUTPUT(<< "Loaded " << images.size() << " images. Remembering properties of the first one for later comparison.")
  mitk::Image::Pointer firstImage = images.front();

  mitk::PropertyList::Pointer originalProperties = firstImage->GetPropertyList(); // save the original
  firstImage->SetPropertyList( mitk::PropertyList::New() ); // clear image properties
  // what about DEFAULT properties? currently ONLY nodes get default properties

  // load for a second time, this time provide the image volume as a pointer
  // expectation is that the reader will provide the same properties to this image (without actually loading a new mitk::Image)

  // !!! we expect that this tests get a list of files that load as ONE SINGLE mitk::Image!
  MITK_TEST_CONDITION_REQUIRED( images.size() == 1, "Not more than 1 images loaded." );
  // otherwise, we would need to determine the correct set of files here
  MITK_TEST_OUTPUT(<< "Generating properties via reader. Comparing new properties to previously loaded version.")
  mitk::Image::Pointer reloadedImage = loader.DecorateVerifyCachedImage(files, firstImage);
  MITK_TEST_CONDITION_REQUIRED(reloadedImage.IsNotNull(), "Reader was able to property-decorate image.");

  mitk::PropertyList::Pointer regeneratedProperties = reloadedImage->GetPropertyList(); // get the version of the second load attempt

  bool listsAreEqual = VerifyPropertyListsEquality(regeneratedProperties, originalProperties);
  MITK_TEST_CONDITION(listsAreEqual, "DICOM file reader generates a valid property list when provided a pre-loaded image");


  // test again, this time provide a tag cache.
  // expectation is, that an empty tag cache will lead to NO image
  mitk::DICOMTagCache::Pointer tagCache; // empty
  MITK_TEST_OUTPUT(<< "Generating properties via reader. Comparing new properties to previously loaded version.")
  firstImage->SetPropertyList( mitk::PropertyList::New() ); // clear image properties
  reloadedImage = loader.DecorateVerifyCachedImage(files, tagCache, firstImage);
  MITK_TEST_CONDITION_REQUIRED(reloadedImage.IsNull(), "Reader was able to detect missing tag-cache.");

  MITK_TEST_END()
}

