/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include <mitkDataNode.h>
#include <mitkIOUtil.h>
#include <mitkInteractionTestHelper.h>
#include <mitkLabelSetImage.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingConfig.h>
#include <mitkToolManager.h>

#include <vtkDebugLeaks.h>

class mitkToolInteractionTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkToolInteractionTestSuite);

  /// \todo Fix VTK memory leaks. Bug 18098.
  vtkDebugLeaks::SetExitError(0);

  /* ####### example ######

  MITK_TEST(AddToolInteraction_4D_Test);
  #########################*/
  /// \todo fix crash when wipe tool test is called after another test
  // MITK_TEST(WipeToolInteractionTest);
  // BUG 19274 - working if recorded again
  // MITK_TEST(AddToolInteractionTest);
  // MITK_TEST(SubtractToolInteractionTest);
  // MITK_TEST(PaintToolInteractionTest);
  // MITK_TEST(RegionGrowingToolInteractionTest);
  // MITK_TEST(CorrectionToolInteractionTest);
  // MITK_TEST(EraseToolInteractionTest);
  // MITK_TEST(FillToolInteractionTest);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ToolManager;

public:
  int GetToolIdByToolName(const std::string &toolName)
  {
    // find tool from toolname
    int numberOfTools = m_ToolManager->GetTools().size();
    int toolId = 0;
    for (; toolId < numberOfTools; ++toolId)
    {
      mitk::Tool *currentTool = m_ToolManager->GetToolById(toolId);
      if (toolName.compare(currentTool->GetNameOfClass()) == 0)
      {
        return toolId;
      }
    }

    return -1;
  }

  void RunTestWithParameters(const std::string &patientImagePath,
                             const std::string &referenceSegmentationImage,
                             const std::string &toolName,
                             const std::string &interactionPattern,
                             unsigned int timestep = 0,
                             const std::string &preSegmentationImagePath = std::string())
  {
    // Create test helper to initialize all necessary objects for interaction
    mitk::InteractionTestHelper interactionTestHelper(GetTestDataFilePath(interactionPattern));

    // Use data storage of test helper
    m_DataStorage = interactionTestHelper.GetDataStorage().GetPointer();

    // create ToolManager
    m_ToolManager = mitk::ToolManager::New(m_DataStorage);
    m_ToolManager->InitializeTools();
    m_ToolManager->RegisterClient(); // This is needed because there must be at least one registered. Otherwise tools
                                     // can't be activated.

    // Load patient image
    mitk::Image::Pointer patientImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath(patientImagePath));
    CPPUNIT_ASSERT(patientImage.IsNotNull());
    mitk::DataNode::Pointer patientImageNode = mitk::DataNode::New();
    patientImageNode->SetData(patientImage);

    // Activate tool to work with
    int toolID = GetToolIdByToolName(toolName);
    mitk::Tool *tool = m_ToolManager->GetToolById(toolID);

    CPPUNIT_ASSERT(tool != nullptr);

    // Create empty segmentation working image
    mitk::DataNode::Pointer workingImageNode = mitk::DataNode::New();
    const std::string organName = "test";
    mitk::Color color; // actually it dosn't matter which color we are using
    color.SetRed(1);   // but CreateEmptySegmentationNode expects a color parameter
    color.SetGreen(0);
    color.SetBlue(0);
    if (preSegmentationImagePath.empty())
    {
      workingImageNode = tool->CreateEmptySegmentationNode(patientImage, organName, color);
    }
    else
    {
      mitk::Image::Pointer preSegmentation = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath(preSegmentationImagePath));
      workingImageNode = tool->CreateSegmentationNode(preSegmentation, organName, color);
    }

    CPPUNIT_ASSERT(workingImageNode.IsNotNull());
    CPPUNIT_ASSERT(workingImageNode->GetData() != nullptr);

    // add images to datastorage
    interactionTestHelper.AddNodeToStorage(patientImageNode);
    interactionTestHelper.AddNodeToStorage(workingImageNode);

    // set reference and working image
    m_ToolManager->SetWorkingData(workingImageNode);
    m_ToolManager->SetReferenceData(patientImageNode);

    // set time step
    interactionTestHelper.SetTimeStep(timestep);

    // load interaction events
    m_ToolManager->ActivateTool(toolID);

    CPPUNIT_ASSERT(m_ToolManager->GetActiveTool() != nullptr);

    // Start Interaction
    interactionTestHelper.PlaybackInteraction();

    // load reference segmentation image
    mitk::Image::Pointer segmentationReferenceImage =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath(referenceSegmentationImage));

    mitk::Image::Pointer currentSegmentationImage = mitk::Image::New();
    currentSegmentationImage = dynamic_cast<mitk::Image *>(workingImageNode->GetData());

    // compare reference with interaction result
    MITK_ASSERT_EQUAL(segmentationReferenceImage, currentSegmentationImage, "Reference equals interaction result.");
  }

  void setUp() {}
  void tearDown()
  {
    m_ToolManager->ActivateTool(-1);
    m_ToolManager = nullptr;
  }

  void AddToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd",
                          "InteractionTestData/ReferenceData/SegmentationInteractor_AddContourTool.nrrd",
                          "AddContourTool",
                          "InteractionTestData/Interactions/SegmentationInteractor_AddTool.xml");
  }

  void SubtractToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd",
                          "InteractionTestData/ReferenceData/SegmentationInteractor_SubtractContourTool.nrrd",
                          "SubtractContourTool",
                          "InteractionTestData/Interactions/SegmentationInteractor_SubtractTool.xml");
  }

  void PaintToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd",
                          "InteractionTestData/ReferenceData/SegmentationInteractor_DrawPaintbrushTool.nrrd",
                          "DrawPaintbrushTool",
                          "InteractionTestData/Interactions/SegmentationInteractor_PaintTool.xml");
  }

  void WipeToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd",
                          "InteractionTestData/ReferenceData/SegmentationInteractor_ErasePaintbrushTool.nrrd",
                          "ErasePaintbrushTool",
                          "InteractionTestData/Interactions/SegmentationInteractor_WipeTool.xml");
  }

  void RegionGrowingToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd",
                          "InteractionTestData/ReferenceData/SegmentationInteractor_RegionGrowingTool.nrrd",
                          "RegionGrowingTool",
                          "InteractionTestData/Interactions/SegmentationInteractor_RegionGrowingTool.xml");
  }

  void CorrectionToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd",
                          "InteractionTestData/ReferenceData/SegmentationInteractor_CorrectorTool2D.nrrd",
                          "CorrectorTool2D",
                          "InteractionTestData/Interactions/SegmentationInteractor_CorrectionTool.xml");
  }

  void EraseToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd",
                          "InteractionTestData/ReferenceData/SegmentationInteractor_EraseRegionTool.nrrd",
                          "EraseRegionTool",
                          "InteractionTestData/Interactions/SegmentationInteractor_EraseTool.xml",
                          0,
                          "InteractionTestData/ReferenceData/SegmentationInteractor_AddContourTool.nrrd");
  }

  void FillToolInteractionTest()
  {
    RunTestWithParameters("Pic3D.nrrd",
                          "InteractionTestData/ReferenceData/SegmentationInteractor_FillRegionTool.nrrd",
                          "FillRegionTool",
                          "InteractionTestData/Interactions/SegmentationInteractor_FillTool.xml",
                          0,
                          "InteractionTestData/InputData/SegmentationInteractor_FillTool_input.nrrd");
  }

  /*############ example ###################
    void AddToolInteraction_4D_Test()
    {

      RunTestWithParameters("US4DCyl.nrrd", "Segmentation/ReferenceSegmentations/AddTool_4D.nrrd", "AddContourTool",
  "Segmentation/InteractionPatterns/AddTool_4D.xml", 1);
    }
  #########################################*/
};

MITK_TEST_SUITE_REGISTRATION(mitkToolInteraction)
