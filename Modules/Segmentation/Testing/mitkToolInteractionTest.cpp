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
#include <mitkStandaloneDataStorage.h>
#include <mitkToolManager.h>
#include <mitkGlobalInteraction.h>
#include <mitkDataNode.h>


class mitkToolInteractionTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkToolInteractionTestSuite);
  MITK_TEST(AddToolInteractionTest);
  CPPUNIT_TEST_SUITE_END();


private:
//  mitk::DataNode::Pointer testPointSetNode;
  mitk::InteractionTestHelper m_InteractionTestHelper;
  mitk::DataStorage* m_DataStorage;
  mitk::ToolManager::Pointer m_ToolManager;

public:

  int GetToolIDFromToolName (const std::string& toolName)
  {
    //find tool from toolname
    int numberOfTools = m_ToolManager->GetTools().size();
    int toolId = 0;
    for(; toolId < numberOfTools; ++toolId)
    {
      mitk::Tool* currentTool = m_ToolManager->GetToolById(toolId);
      if(toolName.compare( currentTool->GetNameOfClass() ) == 0)
      {
        return toolId;
      }
    }
    return -1;
  }

  void RunTestWithParameters(const std::string& patientImagePath, const std::string& referenceSegmentationImage,
                             const std::string& toolName, const std::string& interactionPattern)
  {
    //Load patient image
    mitk::Image::Pointer patientImage = mitk::IOUtil::LoadImage(GetTestDataFilePath(patientImagePath));
    CPPUNIT_ASSERT(patientImage.IsNotNull());
    mitk::DataNode::Pointer patientImageNode = mitk::DataNode::New();
    patientImageNode->SetData(patientImage);

    //Activate tool to work with
    int toolID = GetToolIDFromToolName(toolName);
    mitk::Tool* tool = m_ToolManager->GetToolById(toolID);

    //Create empty segmentation working image
    mitk::DataNode::Pointer workingImageNode = mitk::DataNode::New();
    const std::string organName = "test";
    mitk::Color color;//actually it dosn't matter which color we are using
    color.SetRed(1);  //but CreateEmptySegmentationNode expects a color parameter
    color.SetGreen(0);
    color.SetBlue(0);
    workingImageNode = tool->CreateEmptySegmentationNode(patientImage, organName, color);

    //add images to datastorage
    m_InteractionTestHelper.AddNodeToStorage(patientImageNode);
    m_InteractionTestHelper.AddNodeToStorage(workingImageNode);

    //set reference and working image
    m_ToolManager->SetWorkingData(workingImageNode);
    m_ToolManager->SetReferenceData(patientImageNode);

    //load interaction events
    m_ToolManager->ActivateTool(toolID);
    m_InteractionTestHelper.LoadInteraction(GetTestDataFilePath(interactionPattern));

    //Start Interaction
    m_InteractionTestHelper.PlaybackInteraction();

    //load reference segmentation image
    mitk::Image::Pointer segmentationReferenceImage = mitk::IOUtil::LoadImage(GetTestDataFilePath(referenceSegmentationImage));
    mitk::Image* currentSegmentationImage = dynamic_cast<mitk::Image*>(workingImageNode->GetData());

    //compare reference with interaction result
    MITK_ASSERT_EQUAL(segmentationReferenceImage.GetPointer(), currentSegmentationImage, "Reference equals interaction result." );
  }

  void setUp()
  {
    //Create test helper to initialize all necessary objects for interaction
    m_InteractionTestHelper = mitk::InteractionTestHelper();
    m_DataStorage = m_InteractionTestHelper.GetDataStorage().GetPointer();

    //create ToolManager
    m_ToolManager = mitk::ToolManager::New(m_DataStorage);
    m_ToolManager->InitializeTools();
    m_ToolManager->RegisterClient();//This is needed because there must be at least one registered. Otherwise tools can't be activated.

  }

  void tearDown()
  {
    m_ToolManager->ActivateTool(-1);
    m_ToolManager = NULL;
  }

  void AddToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd", "Segmentation/ReferenceSegmentations/AddTool.nrrd", "AddContourTool", "Segmentation/InteractionPatterns/AddTool.xml");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkToolInteraction)
