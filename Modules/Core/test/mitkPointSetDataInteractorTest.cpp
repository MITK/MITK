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
#include "mitkInteractionTestHelper.h"
#include <mitkPointSet.h>
#include <mitkPointSetDataInteractor.h>

#include <vtkDebugLeaks.h>

class mitkPointSetDataInteractorTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPointSetDataInteractorTestSuite);

  /// \todo Fix VTK memory leaks. Bug 18144.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(AddPointInteraction);
  MITK_TEST(MoveDeletePointInteraction);
  //MITK_TEST(RotatedPlanesInteraction);
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
    std::string referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/TestAddPoints.mps");

    //Path to the interaction xml file
    std::string interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/TestAddPoints.xml");

    std::string pic3D = GetTestDataFilePath("Pic3D.nrrd");
    mitk::Image::Pointer referenceImage = mitk::IOUtil::LoadImage(pic3D);
    mitk::DataNode::Pointer refDN = mitk::DataNode::New();
    refDN->SetData(referenceImage);


    //Create test helper to initialize all necessary objects for interaction
    mitk::InteractionTestHelper interactionTestHelper(interactionXmlPath);

    //Add our test node to the DataStorage of our test helper
    interactionTestHelper.AddNodeToStorage(m_TestPointSetNode);
    interactionTestHelper.AddNodeToStorage(refDN);

    //Start Interaction
    interactionTestHelper.PlaybackInteraction();

    //Load the reference PointSet
    mitk::PointSet::Pointer referencePointSet = mitk::IOUtil::LoadPointSet(referencePointSetPath);

    //Compare reference with the result of the interaction. Last parameter (false) is set to ignore the geometries. They are not stored in a file and therefore not equal.
    CPPUNIT_ASSERT_MESSAGE("", mitk::Equal(referencePointSet, m_TestPointSet, .001, true, false));

  }

  void RotatedPlanesInteraction()
  {

      //Path to the reference PointSet
      std::string referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/PointSetDataInteractor_PointsAdd2d3d.mps");

      //Path to the interaction xml file
      std::string interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/PointSetDataInteractor_PointsAdd2d3d.xml");

      std::string pic3D = GetTestDataFilePath("Pic3D.nrrd");
      mitk::Image::Pointer referenceImage = mitk::IOUtil::LoadImage(pic3D);
      mitk::DataNode::Pointer refDN = mitk::DataNode::New();
      refDN->SetData(referenceImage);


      //Create test helper to initialize all necessary objects for interaction
      mitk::InteractionTestHelper interactionTestHelper(interactionXmlPath);

      //Add our test node to the DataStorage of our test helper
      interactionTestHelper.AddNodeToStorage(m_TestPointSetNode);
      interactionTestHelper.AddNodeToStorage(refDN);

      //Start Interaction
      interactionTestHelper.PlaybackInteraction();

      //Load the reference PointSet
      mitk::PointSet::Pointer referencePointSet = mitk::IOUtil::LoadPointSet(referencePointSetPath);

      //Compare reference with the result of the interaction. Last parameter (false) is set to ignore the geometries. They are not stored in a file and therefore not equal.
      CPPUNIT_ASSERT_MESSAGE("", mitk::Equal(referencePointSet, m_TestPointSet, .001, true, false));

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

  void MoveDeletePointInteraction()
  {
      //Path to the reference PointSet
      std::string referencePointSetPath = GetTestDataFilePath("InteractionTestData/ReferenceData/TestMoveRemovePoints.mps");

      //Path to the interaction xml file
      std::string interactionXmlPath = GetTestDataFilePath("InteractionTestData/Interactions/TestMoveRemovePoints.xml");

      std::string pic3D = GetTestDataFilePath("Pic3D.nrrd");
      mitk::Image::Pointer referenceImage = mitk::IOUtil::LoadImage(pic3D);
      mitk::DataNode::Pointer refDN = mitk::DataNode::New();
      refDN->SetData(referenceImage);


      //Create test helper to initialize all necessary objects for interaction
      mitk::InteractionTestHelper interactionTestHelper(interactionXmlPath);

      //Add our test node to the DataStorage of our test helper
      interactionTestHelper.AddNodeToStorage(m_TestPointSetNode);
      interactionTestHelper.AddNodeToStorage(refDN);

      //Start Interaction
      interactionTestHelper.PlaybackInteraction();

      //Load the reference PointSet
      mitk::PointSet::Pointer referencePointSet = mitk::IOUtil::LoadPointSet(referencePointSetPath);

      //Compare reference with the result of the interaction. Last parameter (false) is set to ignore the geometries. They are not stored in a file and therefore not equal.
      CPPUNIT_ASSERT_MESSAGE("", mitk::Equal(referencePointSet, m_TestPointSet, .001, true, false));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetDataInteractor)
