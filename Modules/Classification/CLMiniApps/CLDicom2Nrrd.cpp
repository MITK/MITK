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

#include "mitkDicomSeriesReader.h"
#include "mitkProperties.h"

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Dicom Loader");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::InputDirectory, "Input folder:", "Input folder",us::Any(),false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output file:", "Output file",us::Any(),false);


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputFolder = us::any_cast<std::string>(parsedArgs["input"]);
  std::string outFileName = us::any_cast<std::string>(parsedArgs["output"]);

  //check if DICOMTags have been set as property for mitk::Image
  mitk::DicomSeriesReader::FileNamesGrouping seriesInFiles = mitk::DicomSeriesReader::GetSeries( inputFolder );
  std::list<mitk::Image::Pointer> images;

  // TODO sort series UIDs, implementation of map iterator might differ on different platforms (or verify this is a standard topic??)
  for (mitk::DicomSeriesReader::FileNamesGrouping::const_iterator seriesIter = seriesInFiles.begin();
       seriesIter != seriesInFiles.end();
       ++seriesIter)
  {
    mitk::Image::Pointer image = seriesIter->second->GetImage();
    if (image.IsNotNull())
    {
      images.push_back( image );
    }
  }

  // WARN: EXPECT ONLY ONE ITEM PER FOLDER
  for ( std::list<mitk::Image::Pointer>::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter )
  {
    const mitk::Image::Pointer image = *imageIter;
    mitk::IOUtil::SaveImage(image,outFileName);
  }
  return EXIT_SUCCESS;
}
