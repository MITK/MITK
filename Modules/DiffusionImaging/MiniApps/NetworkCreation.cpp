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

// std includes
#include <string>

// CTK includes
#include "ctkCommandLineParser.h"

// MITK includes
#include <mitkBaseDataIOFactory.h>
#include "mitkConnectomicsNetworkCreator.h"
#include <mitkCoreObjectFactory.h>

int NetworkCreation(int argc, char* argv[])
{
  ctkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("fiberImage", "f", ctkCommandLineParser::File, "Input image", "input fiber image (.fib)", us::Any(), false);
  parser.addArgument("parcellation", "p", ctkCommandLineParser::File, "Parcellation image", "parcellation image", us::Any(), false);
  parser.addArgument("outputNetwork", "o", ctkCommandLineParser::String, "Output network", "where to save the output (.cnf)", us::Any(), false);

  parser.addArgument("radius", "r", ctkCommandLineParser::Int, "Radius", "Search radius in mm", 15, true);
  parser.addArgument("noCenterOfMass", "com", ctkCommandLineParser::Bool, "No center of mass", "Do not use center of mass for node positions");

  parser.setCategory("Connectomics");
  parser.setTitle("Network Creation");
  parser.setDescription("");
  parser.setContributor("MBI");

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  //default values
  int searchRadius( 15 );
  bool noCenterOfMass( false );

  // parse command line arguments
  std::string fiberFilename = us::any_cast<std::string>(parsedArgs["fiberImage"]);
  std::string parcellationFilename = us::any_cast<std::string>(parsedArgs["parcellation"]);
  std::string outputFilename = us::any_cast<std::string>(parsedArgs["outputNetwork"]);

  if (parsedArgs.count("radius"))
    searchRadius = us::any_cast<int>(parsedArgs["radius"]);


  if (parsedArgs.count("noCenterOfMass"))
    noCenterOfMass = us::any_cast<bool>(parsedArgs["noCenterOfMass"]);

  try
  {

    const std::string s1="", s2="";

    // load fiber image
    std::vector<mitk::BaseData::Pointer> fiberInfile =
      mitk::BaseDataIO::LoadBaseDataFromFile( fiberFilename, s1, s2, false );
    if( fiberInfile.empty() )
    {
      std::string errorMessage = "Fiber Image at " + fiberFilename + " could not be read. Aborting.";
      MITK_ERROR << errorMessage;
      return EXIT_FAILURE;
    }
    mitk::BaseData* fiberBaseData = fiberInfile.at(0);
    mitk::FiberBundleX* fiberBundle = dynamic_cast<mitk::FiberBundleX*>( fiberBaseData );

    // load parcellation
    std::vector<mitk::BaseData::Pointer> parcellationInFile =
      mitk::BaseDataIO::LoadBaseDataFromFile( parcellationFilename, s1, s2, false );
    if( parcellationInFile.empty() )
    {
      std::string errorMessage = "Parcellation at " + parcellationFilename + " could not be read. Aborting.";
      MITK_ERROR << errorMessage;
      return EXIT_FAILURE;
    }
    mitk::BaseData* parcellationBaseData = parcellationInFile.at(0);
    mitk::Image* parcellationImage = dynamic_cast<mitk::Image*>( parcellationBaseData );



    // do creation
    mitk::ConnectomicsNetworkCreator::Pointer connectomicsNetworkCreator = mitk::ConnectomicsNetworkCreator::New();
    connectomicsNetworkCreator->SetSegmentation( parcellationImage );
    connectomicsNetworkCreator->SetFiberBundle( fiberBundle );
    if( !noCenterOfMass )
    {
      connectomicsNetworkCreator->CalculateCenterOfMass();
    }
    connectomicsNetworkCreator->SetEndPointSearchRadius( searchRadius );
    connectomicsNetworkCreator->CreateNetworkFromFibersAndSegmentation();


    mitk::ConnectomicsNetwork::Pointer network = connectomicsNetworkCreator->GetNetwork();

    MITK_INFO << "searching writer";
    mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
    for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
    {
      if ( (*it)->CanWriteBaseDataType(network.GetPointer()) )
      {
        MITK_INFO << "writing";
        (*it)->SetFileName( outputFilename.c_str() );
        (*it)->DoWrite( network.GetPointer() );
      }
    }

    return EXIT_SUCCESS;
  }
  catch (itk::ExceptionObject e)
  {
    MITK_INFO << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    MITK_INFO << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    MITK_INFO << "ERROR!?!";
    return EXIT_FAILURE;
  }
  MITK_INFO << "DONE";
  return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(NetworkCreation);
