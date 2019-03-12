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
#include <string>

// CTK includes
#include "mitkCommandLineParser.h"

// MITK includes
#include "mitkConnectomicsNetworkCreator.h"
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Network Creation");
  parser.setCategory("Connectomics");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "f", mitkCommandLineParser::String, "Input Tractogram", "input tractogram (.fib)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "p", mitkCommandLineParser::String, "Parcellation", "parcellation image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output network", "where to save the output (.cnf; .mat)", us::Any(), false);

  parser.addArgument("noCenterOfMass", "", mitkCommandLineParser::Bool, "No center of mass", "Do not use center of mass for node positions");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  //default values
  bool noCenterOfMass( false );

  // parse command line arguments
  std::string fiberFilename = us::any_cast<std::string>(parsedArgs["f"]);
  std::string parcellationFilename = us::any_cast<std::string>(parsedArgs["p"]);
  std::string outputFilename = us::any_cast<std::string>(parsedArgs["o"]);

  if (parsedArgs.count("noCenterOfMass"))
    noCenterOfMass = us::any_cast<bool>(parsedArgs["noCenterOfMass"]);

  try
  {
    // load fiber image
    std::vector<mitk::BaseData::Pointer> fiberInfile = mitk::IOUtil::Load( fiberFilename);

    if( fiberInfile.empty() )
    {
      std::string errorMessage = "Fiber Image at " + fiberFilename + " could not be read. Aborting.";
      MITK_ERROR << errorMessage;
      return EXIT_FAILURE;
    }
    mitk::BaseData* fiberBaseData = fiberInfile.at(0);
    mitk::FiberBundle* fiberBundle = dynamic_cast<mitk::FiberBundle*>( fiberBaseData );

    // load parcellation
    std::vector<mitk::BaseData::Pointer> parcellationInFile =
        mitk::IOUtil::Load( parcellationFilename);
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
    connectomicsNetworkCreator->SetMappingStrategy(mitk::ConnectomicsNetworkCreator::MappingStrategy::EndElementPosition);
    connectomicsNetworkCreator->CreateNetworkFromFibersAndSegmentation();


    mitk::ConnectomicsNetwork::Pointer network = connectomicsNetworkCreator->GetNetwork();
    mitk::IOUtil::Save(network.GetPointer(), outputFilename );

    return EXIT_SUCCESS;
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  std::cout << "DONE";
  return EXIT_SUCCESS;
}
