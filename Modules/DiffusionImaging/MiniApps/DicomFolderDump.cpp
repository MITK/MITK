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


#include "MiniAppManager.h"

#include "mitkDicomSeriesReader.h"
#include "mitkProperties.h"
// CTK
#include "ctkCommandLineParser.h"
#include "mitkIOUtil.h"


int DicomFolderDump(int argc, char* argv[])
{
  ctkCommandLineParser parser;
  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",ctkCommandLineParser::Bool, "Show this help text");
  parser.addArgument("xml", "x",ctkCommandLineParser::Bool, "Print a XML description of this modules command line interface");
  parser.addArgument("input", "i", ctkCommandLineParser::String, "Input folder",us::Any(),false);
  parser.addArgument("output", "o", ctkCommandLineParser::String, "Output folder (ending with /)",us::Any(),false);
  parser.addArgument("filename", "f", ctkCommandLineParser::String, "Output filename (incl. .nrrd)",us::Any(),false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);


  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputFolder = us::any_cast<string>(parsedArgs["input"]);
  std::string outputFolder = us::any_cast<string>(parsedArgs["output"]);
  std::string outFileName = us::any_cast<string>(parsedArgs["filename"]);

  //check if DICOMTags have been set as property for mitk::Image
  mitk::DicomSeriesReader::FileNamesGrouping seriesInFiles = mitk::DicomSeriesReader::GetSeries( inputFolder, true );
  std::list<mitk::Image::Pointer> images;
  std::map<mitk::Image::Pointer, mitk::DicomSeriesReader::StringContainer> fileMap;

  // TODO sort series UIDs, implementation of map iterator might differ on different platforms (or verify this is a standard topic??)
  for (mitk::DicomSeriesReader::FileNamesGrouping::const_iterator seriesIter = seriesInFiles.begin();
       seriesIter != seriesInFiles.end();
       ++seriesIter)
  {
    mitk::DicomSeriesReader::StringContainer files = seriesIter->second.GetFilenames();

    mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries( files );

    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );

      images.push_back( image );
      fileMap.insert( std::pair<mitk::Image::Pointer, mitk::DicomSeriesReader::StringContainer>(image,files));
    }
  }

  // WARN: EXPECT ONLY ONE ITEM PER FOLDER
  for ( std::list<mitk::Image::Pointer>::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter )
  {
    const mitk::Image::Pointer image = *imageIter;
    mitk::IOUtil::SaveImage(image,outputFolder + outFileName);
  }
  return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(DicomFolderDump);
