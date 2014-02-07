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

#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>

#include <mitkIOUtil.h>
#include <mitkInteractionTestHelper.h>
#include <mitkPointSet.h>
#include <mitkPointSetDataInteractor.h>


class mitkPointSetDataInteractorTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPointSetDataInteractorTestSuite);
  MITK_TEST(TestAddPointInteraction);
  CPPUNIT_TEST_SUITE_END();


public:

  void setUp()
  {

  }

  void TestAddPointInteraction()
  {
    //Path to the reference PointSet
    std::string referencePointSetPath = GetTestDataFilePath("/Users/schroedt/Desktop/pointsetTestRef.mps");

    //Path to the interaction xml file
    std::string interactionXmlPath = "/Users/schroedt/Desktop/pointsetTest.xml";

    //execute
    CPPUNIT_ASSERT(TestPointSetInteraction(interactionXmlPath, referencePointSetPath));
  }

private:

  bool TestPointSetInteraction(std::string interactionXmlPath, std::string referencePointSetPath)
  {
    //Create DataNode as a container for our PointSet to be tested
    mitk::DataNode::Pointer testPointSetNode = mitk::DataNode::New();

    // Create PointSetData Interactor
    mitk::PointSetDataInteractor::Pointer dataInteractor = mitk::PointSetDataInteractor::New();
    // Load the according state machine for regular point set interaction
    dataInteractor->LoadStateMachine("PointSet.xml");
    // Set the configuration file that defines the triggers for the transitions
    dataInteractor->SetEventConfig("PointSetConfig.xml");
    // set the DataNode (which already is added to the DataStorage)
    dataInteractor->SetDataNode(testPointSetNode);

    //Create new PointSet which will receive the interaction input
    mitk::PointSet::Pointer testPointSet = mitk::PointSet::New();
    testPointSetNode->SetData(testPointSet);

    //Create test helper to initialize all necessary objects for interaction
    mitk::InteractionTestHelper interactionTestHelper(interactionXmlPath);
    //Add our test node to the DataStorage of our test helper
    interactionTestHelper.AddNodeToStorage(testPointSetNode);
    //Start Interaction
    interactionTestHelper.PlaybackInteraction();

    //Load the reference PointSet
    mitk::PointSet::Pointer referencePointSet = mitk::IOUtil::LoadPointSet(referencePointSetPath);

    //Compare reference with the result of the interaction
    return mitk::Equal(testPointSet.GetPointer(), referencePointSet.GetPointer(), mitk::eps, false);
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetDataInteractor)
