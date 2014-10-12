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

/**
  \file mitkdump.cpp

  \brief Commandline application to see what DICOMFileReaderSelector would produce from a set of files.

  Usage:
  \verbatim
    mitkdump [-v] file1 [... fileN]

    -v      output more details on commandline
    fileN   DICOM file
  \endverbatim

  The application will ask a DICOMFileReaderSelector to analyze the files given as file1 .. fileN.
  Once the reader with the least number of files is selected, this result is printed to commandline.

  If the "-v" flag is used (as a first parameter), the output will contain details about filenames,
  which can make the output considerably harder to read.

  Output is also written to a log file of name "%gt;datetime-stamp%lt;_dir_&gt;directory-name&lt;.mitkdump
*/

#include "mitkDICOMFileReaderSelector.h"
#include "mitkDICOMReaderConfigurator.h"
#include "mitkDICOMImageFrameInfo.h"

using mitk::DICOMTag;

std::string buildDateString()
{
  std::time_t rawtime;
  std::tm* timeinfo;
  char buffer [80];

  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  std::strftime(buffer,80,"%Y%m%d-%H%M%S",timeinfo);

  return std::string(buffer);
}

void gen_random(char *s, const int len)
{
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  for (int i = 0; i < len; ++i)
  {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
}

std::string gen_random(const int len)
{
  if (len > 0 && len < 100)
  {
    char retval[100];
    gen_random(retval, len);
    return std::string(retval);
  }
  else
  {
    return std::string("");
  }
}

std::string removeUnsafeChars(const std::string& str)
{
  std::string retval;
  for(std::string::const_iterator it = str.begin(); it != str.end(); ++it)
  {
    const char& c = *it;
    if (   (c >= '0' && c <= '9')
        || (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z')
        || (c == '.')
        || (c == '-')
        || (c == '_')
       )
    {
      retval += c;
    }
  }
  return retval;
}

std::string extractDirString(const std::string& dirString)
{
  std::string wholeprefix = dirString.substr(0, dirString.find_last_of("/\\"));
  std::string lastDirectoryPart = wholeprefix.substr(wholeprefix.find_last_of("/\\")+1);
  std::string cleanLastDirectoryPart = removeUnsafeChars(lastDirectoryPart);
  if (!cleanLastDirectoryPart.empty())
  {
    return cleanLastDirectoryPart;
  }
  else
  {
    std::stringstream emptydirname;
    emptydirname << "noname_" << gen_random(6);

    return emptydirname.str();
  }
}

int main(int argc, char* argv[])
{
  bool fileDetails(false);
  bool loadimage(false);
  int firstFileIndex = 1;

  // see if we got the '-v' flag to output file details
  if (argc > 1 && std::string(argv[firstFileIndex]) == "-v")
  {
    fileDetails = true;
    ++firstFileIndex;
  }

  // see if we got the '-l' flag
  if (argc > 1 && std::string(argv[firstFileIndex]) == "-l")
  {
    loadimage = true;
    ++firstFileIndex;
  }


  // analyze files from argv
  mitk::StringList inputFiles;
  for (int a = firstFileIndex; a < argc; ++a)
  {
    inputFiles.push_back( std::string(argv[a]) );
  }

  if (inputFiles.empty())
  {
    MITK_INFO << "0 input files given, exiting...";
    return EXIT_SUCCESS;
  }

  mitk::DICOMFileReaderSelector::Pointer configSelector = mitk::DICOMFileReaderSelector::New();
  configSelector->LoadBuiltIn3DConfigs(); // a set of compiled in ressources with standard configurations that work well
  configSelector->SetInputFiles( inputFiles );
  mitk::DICOMFileReader::Pointer reader = configSelector->GetFirstReaderWithMinimumNumberOfOutputImages();
  if (reader.IsNull())
  {
    MITK_ERROR << "Could not configure any DICOM reader.. Exiting...";
    return EXIT_FAILURE;
  }

  // output best reader result
  MITK_INFO << "---- Best reader configuration '" << reader->GetConfigurationLabel() << "' with " << reader->GetNumberOfOutputs() << " outputs";
  if (fileDetails)
  {
    reader->PrintOutputs(std::cout, fileDetails);
  }

  // construct the name of a log file
  std::string datestring = buildDateString();;
  std::string dirString = extractDirString(argv[firstFileIndex]);
  std::string logfilename = datestring + "_dir_" + dirString + ".mitkdump";
  MITK_INFO << "Logfile " << logfilename;

  // write output to file for later analysis
  std::ofstream fs;
  fs.open(logfilename.c_str());
  fs << "---- " << dirString << ": Best reader configuration '" << reader->GetConfigurationLabel() << "' with " << reader->GetNumberOfOutputs() << " outputs" << std::endl;
  reader->PrintOutputs( fs, true); // always verbose in log file
  fs.close();

  // serialize, deserialize, analyze again, verify result!
  mitk::DICOMReaderConfigurator::Pointer serializer = mitk::DICOMReaderConfigurator::New();
  std::string readerSerialization = serializer->CreateConfigStringFromReader(reader.GetPointer());

  bool outputError(false);
  for (unsigned int outputIndex = 0; outputIndex < reader->GetNumberOfOutputs(); ++outputIndex)
  {
    const mitk::DICOMImageBlockDescriptor& outputDescriptor = reader->GetOutput(outputIndex);

    mitk::StringList filenamesOfThisGroup;
    const mitk::DICOMImageFrameList& frames = outputDescriptor.GetImageFrameList();
    for (mitk::DICOMImageFrameList::const_iterator fIter = frames.begin(); fIter != frames.end(); ++fIter)
    {
      filenamesOfThisGroup.push_back( (*fIter)->Filename );
    }

    mitk::DICOMReaderConfigurator::Pointer readerConfigurator = mitk::DICOMReaderConfigurator::New();
    mitk::DICOMFileReader::Pointer dicomReader = readerConfigurator->CreateFromUTF8ConfigString( readerSerialization );
    dicomReader->SetInputFiles( filenamesOfThisGroup );
    dicomReader->AnalyzeInputFiles();
    if (dicomReader->GetNumberOfOutputs() != 1)
    {
      MITK_ERROR << "****** Re-analyzing files of output group " << outputIndex << " yields " << dicomReader->GetNumberOfOutputs() << " groups";
      outputError = true;

      for (mitk::DICOMImageFrameList::const_iterator fIter = frames.begin(); fIter != frames.end(); ++fIter)
      {
        MITK_INFO << "filename group " << outputIndex << ": "  << (*fIter)->Filename;
      }
    }
    else
    {
      MITK_INFO << "Re-analyzing files of output group " << outputIndex << " yields " << dicomReader->GetNumberOfOutputs() << " groups";
    }
  }

  if (outputError)
  {
    std::stringstream es;
    es << "Original reader configuration: " << std::endl;
    reader->PrintConfiguration(es);
    es << std::endl;
    mitk::DICOMReaderConfigurator::Pointer readerConfigurator = mitk::DICOMReaderConfigurator::New();
    mitk::DICOMFileReader::Pointer dicomReader = readerConfigurator->CreateFromUTF8ConfigString( readerSerialization );
    es << "New reader configuration: " << std::endl;
    dicomReader->PrintConfiguration(es);
    es << std::endl;

    es << "Original XML: \n" << readerSerialization << std::endl;
    std::string newSerialization = serializer->CreateConfigStringFromReader(dicomReader.GetPointer());
    es << "New XML: \n" << newSerialization << std::endl;
    MITK_ERROR << es.str();
  }

  if (loadimage)
  {
    MITK_INFO << "Loading...";
    reader->LoadImages();
    mitk::Image::Pointer image = reader->GetOutput(0).GetMitkImage();
    MITK_INFO << "---- Output image:";
    mitk::BaseGeometry::Pointer geo3D = image->GetGeometry();
    if (geo3D.IsNotNull())
    {
      mitk::SlicedGeometry3D::Pointer sg = dynamic_cast<mitk::SlicedGeometry3D*>(geo3D.GetPointer());
      if (sg.IsNotNull())
      {
        unsigned int nos = sg->GetSlices();
        mitk::PlaneGeometry::ConstPointer first = sg->GetPlaneGeometry(0);
        mitk::PlaneGeometry::ConstPointer last = sg->GetPlaneGeometry(nos-1);

        mitk::Point3D firstOrigin = first->GetOrigin();
        mitk::Point3D lastOrigin = last->GetOrigin();
        MITK_INFO << "Geometry says: First slice at " << firstOrigin << ", last slice at " << lastOrigin;

        mitk::StringLookupTableProperty::Pointer sliceLocations =
          dynamic_cast<mitk::StringLookupTableProperty*>( image->GetProperty("dicom.image.0020.1041").GetPointer() );
        if (sliceLocations.IsNotNull())
        {
          std::string firstSliceLocation = sliceLocations->GetValue().GetTableValue(0);
          std::string lastSliceLocation = sliceLocations->GetValue().GetTableValue(nos-1);
          MITK_INFO << "Image properties says: first slice location at " << firstSliceLocation << ", last slice location at " << lastSliceLocation;
        }

        mitk::StringLookupTableProperty::Pointer instanceNumbers =
          dynamic_cast<mitk::StringLookupTableProperty*>( image->GetProperty("dicom.image.0020.0013").GetPointer() );
        if (instanceNumbers.IsNotNull())
        {
          std::string firstInstanceNumber = instanceNumbers->GetValue().GetTableValue(0);
          std::string lastInstanceNumber = instanceNumbers->GetValue().GetTableValue(nos-1);
          MITK_INFO << "Image properties says: first instance number at " << firstInstanceNumber << ", last instance number at " << lastInstanceNumber;
        }
      }
    }
    MITK_INFO << "---- End of output";
  }

  // if we got so far, everything is fine
  return EXIT_SUCCESS;
}
