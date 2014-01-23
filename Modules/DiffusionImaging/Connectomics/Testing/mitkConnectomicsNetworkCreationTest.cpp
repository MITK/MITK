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

// macros
#include "mitkTestingMacros.h"

// std includes
#include <string>

// MITK includes
#include <mitkBaseDataIOFactory.h>
#include "mitkConnectomicsNetworkCreator.h"
#include <mitkCoreObjectFactory.h>


static void CreateNetworkFromFibersAndParcellation(std::string fiberFilename, std::string parcellationFilename, std::string outputFilename, bool testMode, std::string referenceFilename)
{
  const std::string s1="", s2="";

  // load fiber image
  std::vector<mitk::BaseData::Pointer> fiberInfile =
    mitk::BaseDataIO::LoadBaseDataFromFile( fiberFilename, s1, s2, false );
  if( fiberInfile.empty() )
  {
    std::string errorMessage = "Fiber Image at " + parcellationFilename + " could not be read. Aborting.";
    MITK_TEST_CONDITION_REQUIRED( false, errorMessage);
  }
  mitk::BaseData* fiberBaseData = fiberInfile.at(0);
  mitk::FiberBundleX* fiberBundle = dynamic_cast<mitk::FiberBundleX*>( fiberBaseData );

  // load parcellation
  std::vector<mitk::BaseData::Pointer> parcellationInFile =
    mitk::BaseDataIO::LoadBaseDataFromFile( parcellationFilename, s1, s2, false );
  if( parcellationInFile.empty() )
  {
    std::string errorMessage = "Parcellation at " + parcellationFilename + " could not be read. Aborting.";
    MITK_TEST_CONDITION_REQUIRED( false, errorMessage);
  }
  mitk::BaseData* parcellationBaseData = parcellationInFile.at(0);
  mitk::Image* parcellationImage = dynamic_cast<mitk::Image*>( parcellationBaseData );



  // do creation
  mitk::ConnectomicsNetworkCreator::Pointer connectomicsNetworkCreator = mitk::ConnectomicsNetworkCreator::New();
  connectomicsNetworkCreator->SetSegmentation( parcellationImage );
  connectomicsNetworkCreator->SetFiberBundle( fiberBundle );
  connectomicsNetworkCreator->CalculateCenterOfMass();
  connectomicsNetworkCreator->SetEndPointSearchRadius( 15 );
  connectomicsNetworkCreator->CreateNetworkFromFibersAndSegmentation();

  // write if not test mode
  if( !testMode )
  {
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
  }
  else
  {
    // load network
    std::vector<mitk::BaseData::Pointer> referenceFile =
      mitk::BaseDataIO::LoadBaseDataFromFile( referenceFilename, s1, s2, false );
    if( referenceFile.empty() )
    {
      std::string errorMessage = "Reference Network at " + referenceFilename + " could not be read. Aborting.";
      MITK_TEST_CONDITION_REQUIRED( false, errorMessage);
    }
    mitk::BaseData* referenceBaseData = referenceFile.at(0);
    mitk::ConnectomicsNetwork* referenceNetwork = dynamic_cast<mitk::ConnectomicsNetwork*>( referenceBaseData );

    mitk::ConnectomicsNetwork::Pointer network = connectomicsNetworkCreator->GetNetwork();

    MITK_TEST_CONDITION_REQUIRED( mitk::Equal( network.GetPointer(), referenceNetwork, mitk::eps, true), "Comparing created and reference network.");
  }
}


/**
* @brief mitkConnectomicsNetworkCreationTest A test for network creation
*/
int mitkConnectomicsNetworkCreationTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN(mitkConnectomicsNetworkCreationTest);

  bool validNumberOfArguments( (argc == 4) || (argc == 5) );

  if( !validNumberOfArguments )
  {
    std::string errorMessage = "Wrong number of arguments.\nUsage: <fiber-filename> <parcellation-filename> <network-out-filename> [<reference filename>]";
    MITK_TEST_CONDITION_REQUIRED( !validNumberOfArguments, errorMessage);
    return 0;
  }

  std::string fiber_name = argv[1];
  std::string parcellation_name = argv[2];
  std::string network_name = argv[3];

  if(argc == 4)
  {
    CreateNetworkFromFibersAndParcellation(fiber_name, parcellation_name, network_name, false, ""  );
  }
  if(argc == 5)
  {
    std::string reference_name = argv[4];
    CreateNetworkFromFibersAndParcellation(fiber_name, parcellation_name, network_name, true, reference_name  );
  }

  MITK_TEST_END();
}
