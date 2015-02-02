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

std::vector<std::string> LoadDumps(const std::string& fileName)
{
  std::vector<std::string> separatedDumps;

  std::ifstream fileStream( fileName.c_str() );

  std::string buffer;
  std::string line;
  while(fileStream){
    std::getline(fileStream, line);

    if (line.find("-- Image ") == 0)
    {
      // separator: starts a new image block
      if ( !buffer.empty() )
      {
        // unless this is the first block
        separatedDumps.push_back(buffer);
        buffer.clear();
      }
    }
    else
    {
      buffer += line + "\n";
    }
  }
  fileStream.close();

  // eat last image dump
  if ( !buffer.empty() )
  {
    separatedDumps.push_back(buffer);
    buffer.clear();
  }

  return separatedDumps;
}


int main(int argc, char** argv)
{
  /**
    Loads a list of DICOM images, compares generated mitk::Images against stored references.

    first argument: file with reference dumps
    following arguments: file names to load
  */

  if (argc < 2)
  {
    MITK_ERROR << "Usage:";
    MITK_ERROR << "  " << argv[0] << " reference.dump file1 [file2 .. fileN]";
    MITK_ERROR << "  ";
    MITK_ERROR << "  Loads all DICOM images in file1 to fileN as MITK images ";
    MITK_ERROR << "  and compares loaded images against stored expectations (dumps).";
    MITK_ERROR << "  See also DumpDICOMMitkImage (generates dumps)";
    return EXIT_FAILURE;
  }

  mitk::TestDICOMLoading loader;
  mitk::StringList files;

  // load from argv[1]
  // separate at "-- Image n"
  // store in an array of dumps

  std::vector<std::string> expectedDumps = LoadDumps(argv[1]);

  for (int arg = 2; arg < argc; ++arg) files.push_back( argv[arg] );

  mitk::TestDICOMLoading::ImageList images = loader.LoadFiles(files);

  unsigned int imageCounter(0);
  for ( mitk::TestDICOMLoading::ImageList::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter, ++imageCounter )
  {
    std::string imageDump = loader.DumpImageInformation( *imageIter );

    if (imageCounter >= expectedDumps.size())
    {
      MITK_ERROR << "Loader produces more images than expected. Aborting after image " << (imageCounter-1);
      MITK_INFO << "Image " << imageCounter << " loaded as:\n" << imageDump;
      return EXIT_FAILURE;
    }

    bool loadedAsExpected = loader.CompareImageInformationDumps( expectedDumps[imageCounter], imageDump );
    if (loadedAsExpected)
    {
      MITK_INFO << "Image " << imageCounter << " loads as expected.";
    }
    else
    {
      MITK_ERROR << "Image " << imageCounter << " did not load as expected.";
      MITK_INFO << "Expected: \n" << expectedDumps[imageCounter] << "\nGot:\n" << imageDump;
      return EXIT_FAILURE;
    }
  }

  if (images.size() < expectedDumps.size())
  {
    MITK_ERROR << "Loader produced only " << images.size() << " mitk::Images, while test expected " << expectedDumps.size() << " images";
    return EXIT_FAILURE;
  }


  return EXIT_SUCCESS;
}

