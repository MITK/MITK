
#include "mitkTestDICOMLoading.h"

int main(int argc, char** argv)
{
  /**
    Loads a list of DICOM images, compares generated mitk::Images against stored references.

    first argument: file with reference dumps
    following arguments: file names to load
  */
  mitk::TestDICOMLoading loader;
  mitk::TestDICOMLoading::StringContainer files;
    
  // TODO load reference dumps

  for (int arg = 2; arg < argc; ++arg) files.push_back( argv[arg] );
  
  mitk::TestDICOMLoading::ImageList images = loader.LoadFiles(files);

  unsigned int imageCounter(0);
  for ( mitk::TestDICOMLoading::ImageList::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter )
  {
    std::string imageDump = loader.DumpImageInformation( *imageIter );

    // TODO compare against reference
  }
}

