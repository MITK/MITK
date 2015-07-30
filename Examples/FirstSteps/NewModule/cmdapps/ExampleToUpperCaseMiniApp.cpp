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

// std includes
#include <algorithm>
#include <string>

// CTK includes
#include "mitkCommandLineParser.h"

// MITK includes
#include "mitkExampleDataStructure.h"
#include <mitkIOUtil.h>

int main( int argc, char* argv[] )
{
  mitkCommandLineParser parser;

  parser.setTitle( "To Upper Case" );
  parser.setCategory( "MITK-Examples" );
  parser.setDescription( "" );
  parser.setContributor( "MBI" );

  parser.setArgumentPrefix( "--", "-" );
  parser.addArgument(
    "input", "i", mitkCommandLineParser::InputFile, "Input file", "input file (.txt/.example)", us::Any(), false );
  parser.addArgument( "output",
                      "o",
                      mitkCommandLineParser::OutputFile,
                      "Output file",
                      "where to save the output (.txt/.example)",
                      us::Any(),
                      false );

  parser.addArgument(
    "verbose", "v", mitkCommandLineParser::Bool, "Verbose Output", "Whether to produce verbose output" );

  map<string, us::Any> parsedArgs = parser.parseArguments( argc, argv );
  if ( parsedArgs.size() == 0 )
    return EXIT_FAILURE;

  // default values
  bool verbose( false );

  // parse command line arguments
  std::string inFilename = us::any_cast<std::string>( parsedArgs[ "input" ] );
  std::string outFileName = us::any_cast<std::string>( parsedArgs[ "output" ] );

  // if optional flag is present
  if ( parsedArgs.count( "verbose" ) )
    verbose = us::any_cast<bool>( parsedArgs[ "verbose" ] );

  try
  {
    // load file
    std::vector<mitk::BaseData::Pointer> inVector = mitk::IOUtil::Load( inFilename );
    if ( inVector.empty() )
    {
      std::string errorMessage = "File at " + inFilename + " could not be read. Aborting.";
      MITK_ERROR << errorMessage;
      return EXIT_FAILURE;
    }
    mitk::BaseData* inBaseData = inVector.at( 0 );
    mitk::ExampleDataStructure* inExample = dynamic_cast<mitk::ExampleDataStructure*>( inBaseData );

    // do creation
    mitk::ExampleDataStructure::Pointer outExample = mitk::ExampleDataStructure::New();
    std::string data = inExample->GetData();
    std::transform( data.begin(), data.end(), data.begin(), ::toupper );
    outExample->SetData( data );

    std::cout << "searching writer";

    mitk::IOUtil::SaveBaseData( outExample.GetPointer(), outFileName );

    return EXIT_SUCCESS;
  }
  catch ( itk::ExceptionObject e )
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch ( std::exception e )
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch ( ... )
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  std::cout << "DONE";
  return EXIT_SUCCESS;
}
