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
  char retval[len];
  gen_random(retval, len);
  return std::string(retval);
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
  int firstFileIndex = 1;

  // see if we got the '-v' flag to output file details
  if (argc > 1 && std::string(argv[firstFileIndex]) == "-v")
  {
    fileDetails = true;
    ++firstFileIndex;
  }

  // analyze files from argv
  mitk::StringList inputFiles;
  for (int a = firstFileIndex; a < argc; ++a)
  {
    inputFiles.push_back( std::string(argv[a]) );
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
  MITK_INFO << "---- Best reader configuration '" << reader->GetConfigurationLabel() << "'";
  reader->PrintOutputs(std::cout, fileDetails);

  // construct the name of a log file
  std::string datestring = buildDateString();;
  std::string dirString = extractDirString(argv[firstFileIndex]);
  std::string logfilename = datestring + "_dir_" + dirString + ".mitkdump";
  MITK_INFO << "Logfile " << logfilename;

  // write output to file for later analysis
  std::ofstream fs;
  fs.open(logfilename.c_str());
  reader->PrintOutputs( fs, true); // always verbose in log file
  fs.close();

  // if we got so far, everything is fine
  return EXIT_SUCCESS;
}
