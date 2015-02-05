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
#include "mitkImage.h"

int main(int argc, char** argv)
{
  mitk::TestDICOMLoading loader;
  mitk::StringList files;

  for (int arg = 1; arg < argc; ++arg) files.push_back( argv[arg] );

  mitk::TestDICOMLoading::ImageList images = loader.LoadFiles(files);

  // combine individual dumps in a way that VerifyDICOMMitkImageDump is able to separate again.
  // I.e.: when changing this piece of code, always change VerifyDICOMMitkImageDump, too.
  unsigned int imageCounter(0);
  for ( mitk::TestDICOMLoading::ImageList::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter )
  {
    std::cout << "-- Image " << ++imageCounter << "\n";
    std::cout << loader.DumpImageInformation( *imageIter ) << "\n";
  }
}

