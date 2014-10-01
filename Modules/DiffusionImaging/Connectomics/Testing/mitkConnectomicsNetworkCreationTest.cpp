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

// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

// std includes
#include <string>

// MITK includes
#include "mitkConnectomicsNetworkCreator.h"
#include "mitkIOUtil.h"

// VTK includes
#include <vtkDebugLeaks.h>

class mitkConnectomicsNetworkCreationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkConnectomicsNetworkCreationTestSuite);

  /// \todo Fix VTK memory leaks. Bug 18097.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(CreateNetworkFromFibersAndParcellation);
  CPPUNIT_TEST_SUITE_END();

private:

  std::string m_ParcellationPath;
  std::string m_FiberPath;
  std::string m_ReferenceNetworkPath;

public:

  /**
  * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
  */
  void setUp()
  {
    m_ReferenceNetworkPath = GetTestDataFilePath("DiffusionImaging/Connectomics/reference.cnf");
    m_ParcellationPath = GetTestDataFilePath("DiffusionImaging/Connectomics/parcellation.nrrd");
    m_FiberPath = GetTestDataFilePath("DiffusionImaging/Connectomics/fiberBundle.fib");
  }

  void tearDown()
  {
    m_ReferenceNetworkPath = "";
    m_ParcellationPath = "";
    m_FiberPath = "";
  }

  void CreateNetworkFromFibersAndParcellation()
  {
    // load fiber image
    std::vector<mitk::BaseData::Pointer> fiberInfile = mitk::IOUtil::Load( m_FiberPath );
    if( fiberInfile.empty() )
    {
      std::string errorMessage = "Fiber Image at " + m_FiberPath + " could not be read. Aborting.";
      CPPUNIT_ASSERT_MESSAGE( errorMessage, false );
    }
    mitk::BaseData* fiberBaseData = fiberInfile.at(0);
    mitk::FiberBundleX* fiberBundle = dynamic_cast<mitk::FiberBundleX*>( fiberBaseData );

    // load parcellation
    std::vector<mitk::BaseData::Pointer> parcellationInFile = mitk::IOUtil::Load( m_ParcellationPath );
    if( parcellationInFile.empty() )
    {
      std::string errorMessage = "Parcellation at " + m_ParcellationPath + " could not be read. Aborting.";
      CPPUNIT_ASSERT_MESSAGE( errorMessage, false );
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

    // load network
    std::vector<mitk::BaseData::Pointer> referenceFile = mitk::IOUtil::Load( m_ReferenceNetworkPath );
    if( referenceFile.empty() )
    {
      std::string errorMessage = "Reference Network at " + m_ReferenceNetworkPath + " could not be read. Aborting.";
      CPPUNIT_ASSERT_MESSAGE( errorMessage, false );
    }
    mitk::BaseData* referenceBaseData = referenceFile.at(0);
    mitk::ConnectomicsNetwork* referenceNetwork = dynamic_cast<mitk::ConnectomicsNetwork*>( referenceBaseData );

    mitk::ConnectomicsNetwork::Pointer network = connectomicsNetworkCreator->GetNetwork();

    CPPUNIT_ASSERT_MESSAGE( "Comparing created and reference network.", mitk::Equal( network.GetPointer(), referenceNetwork, mitk::eps, true) );

  }
};

MITK_TEST_SUITE_REGISTRATION(mitkConnectomicsNetworkCreation)
