
#include "mitkTestDICOMLoading.h"

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
  mitk::TestDICOMLoading::StringContainer files;
    
  // TODO load reference dumps

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

    bool loadedAsExpected = loader.CompareImageInformationDumps( expectedDumps[imageCounter], imageDump );
    MITK_INFO << "Image " << imageCounter << " loads as expected.";
    // TODO compare against reference
  }

  return EXIT_SUCCESS;
}

