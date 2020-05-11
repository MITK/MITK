/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestDICOMLoading.h"
#include "mitkImage.h"

int main(int argc, char** argv)
{
  mitk::TestDICOMLoading loader;
  mitk::StringList files;

  if (argc < 2)
  {
    std::cerr << "Wrong usage of DumpDICOMMitkImage. Call it like VerifyDICOMMitkImageDump <dumpfile> <dcmfile1> [<dcmfile2> [... <dcmfileN>]].";
    return 1;
  }

  std::string dumpPath = argv[1];

  for (int arg = 2; arg < argc; ++arg) files.push_back( argv[arg] );

  mitk::TestDICOMLoading::ImageList images = loader.LoadFiles(files);

  std::ostringstream sstream;
  // combine individual dumps in a way that VerifyDICOMMitkImageDump is able to separate again.
  // I.e.: when changing this piece of code, always change VerifyDICOMMitkImageDump, too.
  unsigned int imageCounter(0);
  for ( mitk::TestDICOMLoading::ImageList::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter )
  {
    sstream << "-- Image " << ++imageCounter << "\n";
    sstream << loader.DumpImageInformation( *imageIter ) << "\n";
  }
  std::cout << sstream.str();
  std::ofstream out(dumpPath, ios::trunc | ios::out);
  out << sstream.str();
  out.close();

}

