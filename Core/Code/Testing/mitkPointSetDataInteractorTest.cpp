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

#include <vtkDebugLeaks.h>

class mitkPointSetDataInteractorTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPointSetDataInteractorTestSuite);

  /// \todo Fix VTK memory leaks. Bug 18144.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(AddPointInteraction);
  MITK_TEST(DeletePointInteraction);
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

    //Create new PointSet which will receive the interaction input
    m_TestPointSet = mitk::PointSet::New();
    m_TestPointSetNode->SetData(m_TestPointSet);
    // set the DataNode (which already is added to the DataStorage)
    m_DataInteractor->SetDataNode(m_TestPointSetNode);
  }

  void tearDown()
  {
    //destroy all objects
    m_TestPointSetNode->SetDataInteractor(NULL);
    m_TestPointSetNode = NULL;
    m_TestPointSet = NULL;
    m_DataInteractor = NULL;
  }

  void AddPointInteraction()
  {
    //Path to the reference PointSet
    std::string referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/PointSetDataInteractor_add_points_in_2D_ref.mps");

    //Path to the interaction xml file
    std::string interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PointSetDataInteractor_add_points_in_2D.xml");

    //Create test helper to initialize all necessary objects for interaction
    mitk::InteractionTestHelper interactionTestHelper(interactionXmlPath);

    //Add our test node to the DataStorage of our test helper
    interactionTestHelper.AddNodeToStorage(m_TestPointSetNode);

    //Start Interaction
    interactionTestHelper.PlaybackInteraction();

    //Load the reference PointSet
    mitk::PointSet::Pointer referencePointSet = mitk::IOUtil::LoadPointSet(referencePointSetPath);

    //Compare reference with the result of the interaction
    MITK_ASSERT_EQUAL(m_TestPointSet, referencePointSet, "");
  }

  void PlayInteraction( std::string &xmlFile, mitk::DataNode* node )
  {
    mitk::InteractionTestHelper interactionTestHelper( xmlFile );
    interactionTestHelper.AddNodeToStorage( node );
    interactionTestHelper.PlaybackInteraction();
  }

  void EvaluateState( std::string &refPsFile, mitk::PointSet::Pointer ps, int selected )
  {
    mitk::PointSet::Pointer refPs = mitk::IOUtil::LoadPointSet( refPsFile );
    refPs->UpdateOutputInformation();
    ps->UpdateOutputInformation();

    MITK_ASSERT_EQUAL(ps, refPs, "");

    MITK_TEST_CONDITION_REQUIRED(true, "Test against reference point set." );
    MITK_TEST_CONDITION_REQUIRED(ps->GetNumberOfSelected() == 1, "One selected point." );
    MITK_TEST_CONDITION_REQUIRED(ps->GetSelectInfo( selected ) , "Testing if proper point is selected." );
  }

  void SetupInteractor( mitk::PointSetDataInteractor* dataInteractor, mitk::DataNode* node )
  {
    dataInteractor->LoadStateMachine("PointSet.xml");
    dataInteractor->SetEventConfig("PointSetConfig.xml");
    dataInteractor->SetDataNode( node );
  }

  void DeletePointInteraction()
  {
    mitk::PointSetDataInteractor::Pointer dataInteractor = mitk::PointSetDataInteractor::New();
    mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();

    //Path to the reference PointSet
    std::string referencePointSetPath = GetTestDataFilePath("InteractionTestData/InputData/InitPointSet.mps");
    mitk::PointSet::Pointer ps = mitk::IOUtil::LoadPointSet( referencePointSetPath );
    pointSetNode->SetData( ps );

    this->SetupInteractor( dataInteractor, pointSetNode );

    std::string interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PSDataInteractionDel-0_1.xml");
    referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/DataInteractionDel-refPS-0.mps");
    PlayInteraction( interactionXmlPath, pointSetNode );
    EvaluateState( referencePointSetPath, ps, 1 );

    interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PSDataInteractionDel-1_3.xml");
    referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/DataInteractionDel-refPS-1.mps");
    PlayInteraction( interactionXmlPath, pointSetNode );
    EvaluateState( referencePointSetPath, ps, 1 );

    interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PSDataInteractionDel-2_4.xml");
    referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/DataInteractionDel-refPS-2.mps");
    PlayInteraction( interactionXmlPath, pointSetNode );
    EvaluateState( referencePointSetPath, ps, 1 );

    interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PSDataInteractionDel-3_8.xml");
    referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/DataInteractionDel-refPS-3.mps");
    PlayInteraction( interactionXmlPath, pointSetNode );
    EvaluateState( referencePointSetPath, ps, 1 );

    interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PSDataInteractionDel-4_2.xml");
    referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/DataInteractionDel-refPS-4.mps");
    PlayInteraction( interactionXmlPath, pointSetNode );
    EvaluateState( referencePointSetPath, ps, 4 );

    interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PSDataInteractionDel-5_6.xml");
    referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/DataInteractionDel-refPS-5.mps");
    PlayInteraction( interactionXmlPath, pointSetNode );
    EvaluateState( referencePointSetPath, ps, 4 );

    interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PSDataInteractionDel-6_7.xml");
    referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/DataInteractionDel-refPS-6.mps");
    PlayInteraction( interactionXmlPath, pointSetNode );
    EvaluateState( referencePointSetPath, ps, 4 );

    interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PSDataInteractionDel-7_5.xml");
    //referencePointSetPath = GetTestDataFilePath("InteractionTestData/PointSet1.mps");
    PlayInteraction( interactionXmlPath, pointSetNode );

    MITK_TEST_CONDITION_REQUIRED(ps->GetPointSet()->GetNumberOfPoints() == 0, "Empty point set check.");
    MITK_TEST_CONDITION_REQUIRED(ps->GetNumberOfSelected() == 0, "No selected points." );
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetDataInteractor)
