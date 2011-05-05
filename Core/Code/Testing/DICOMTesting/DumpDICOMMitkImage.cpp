#include "mitkTestDICOMLoading.h"

int main(int argc, char** argv)
{
  mitk::TestDICOMLoading loader;
  mitk::TestDICOMLoading::StringContainer files;

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

