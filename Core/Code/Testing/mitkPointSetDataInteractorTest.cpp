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
  MITK_TEST(AddPointInteraction);
  CPPUNIT_TEST_SUITE_END();


private:
  mitk::DataNode::Pointer m_TestPointSetNode;
  mitk::PointSetDataInteractor::Pointer m_DataInteractor;
  mitk::PointSet::Pointer m_TestPointSet;

public:

  void setUp()
  {
    //Create DataNode as a container for our PointSet to be tested
    m_TestPointSetNode = mitk::DataNode::New();

    // Create PointSetData Interactor
    m_DataInteractor = mitk::PointSetDataInteractor::New();
    // Load the according state machine for regular point set interaction
    m_DataInteractor->LoadStateMachine("PointSet.xml");
    // Set the configuration file that defines the triggers for the transitions
    m_DataInteractor->SetEventConfig("PointSetConfig.xml");
    // set the DataNode (which already is added to the DataStorage)
    m_DataInteractor->SetDataNode(m_TestPointSetNode);

    //Create new PointSet which will receive the interaction input
    m_TestPointSet = mitk::PointSet::New();
    m_TestPointSetNode->SetData(m_TestPointSet);
  }

  void tearDown()
  {
    //destroy all objects
    m_TestPointSetNode = NULL;
    m_TestPointSet = NULL;
    m_DataInteractor = NULL;
  }

  void AddPointInteraction()
  {
    //Path to the reference PointSet
    std::string referencePointSetPath = "/Users/schroedt/Desktop/pointsetTestRef.mps";

    //Path to the interaction xml file
    std::string interactionXmlPath = "/Users/schroedt/Desktop/pointsetTest.xml";

    //Create test helper to initialize all necessary objects for interaction
    mitk::InteractionTestHelper interactionTestHelper(interactionXmlPath);

    //Add our test node to the DataStorage of our test helper
    interactionTestHelper.AddNodeToStorage(m_TestPointSetNode);

    //Start Interaction
    interactionTestHelper.PlaybackInteraction();

    //Load the reference PointSet
    mitk::PointSet::Pointer referencePointSet = mitk::IOUtil::LoadPointSet(referencePointSetPath);

    //Compare reference with the result of the interaction
    MITK_ASSERT_EQUAL(m_TestPointSet.GetPointer(), referencePointSet.GetPointer(), "");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetDataInteractor)
