/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkIOUtil.h>
#include <mitkLevelWindowManager.h>
#include <mitkRenderingModeProperty.h>
#include <mitkStandaloneDataStorage.h>

#include <itkEventObject.h>

class mitkLevelWindowManagerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLevelWindowManagerTestSuite);
  MITK_TEST(TestModes);
  MITK_TEST(TestSetModeWithoutDataStorage);
  MITK_TEST(TestSetLevelWindowProperty);
  MITK_TEST(TestVisibilityPropertyChanged);
  MITK_TEST(TestLayerPropertyChanged);
  MITK_TEST(TestImageRenderingModePropertyChanged);
  MITK_TEST(TestSelectedPropertyChanged);
  MITK_TEST(TestDeterministicTopMostSelectedNode);
  MITK_TEST(TestApplyToAllSelectedImages);
  MITK_TEST(TestExplicitImageSticks);
  MITK_TEST(TestExplicitNodeInvisibleThenVisible);
  MITK_TEST(TestModeSticksAcrossNodeAdd);
  MITK_TEST(TestModeSticksAcrossSetDataStorage);
  MITK_TEST(TestRemoveDataNodes);
  MITK_TEST(TestRemoveNodeInSelectedImageMode);
  MITK_TEST(TestRemoveNodeInExplicitImageMode);
  MITK_TEST(TestCurrentNodeAndNameChange);
  MITK_TEST(TestCombinedPropertiesChanged);

  CPPUNIT_TEST_SUITE_END();

private:

  using Mode = mitk::LevelWindowManager::Mode;

  mitk::LevelWindowManager::Pointer m_LevelWindowManager;
  mitk::StandaloneDataStorage::Pointer m_DataManager;

  std::string m_ImagePath1;
  std::string m_ImagePath2;
  std::string m_ImagePath3;

  mitk::DataNode::Pointer m_DataNode1;
  mitk::DataNode::Pointer m_DataNode2;
  mitk::DataNode::Pointer m_DataNode3;

  bool AssertImageForLevelWindowProperty(bool assert1, bool assert2, bool assert3)
  {
    bool imageForLevelWindowProperty1 = false, imageForLevelWindowProperty2 = false, imageForLevelWindowProperty3 = false;

    m_DataNode1->GetBoolProperty("imageForLevelWindow", imageForLevelWindowProperty1);
    m_DataNode2->GetBoolProperty("imageForLevelWindow", imageForLevelWindowProperty2);
    m_DataNode3->GetBoolProperty("imageForLevelWindow", imageForLevelWindowProperty3);

    return (assert1 == imageForLevelWindowProperty1) &&
           (assert2 == imageForLevelWindowProperty2) &&
           (assert3 == imageForLevelWindowProperty3);
  }

  bool AssertLevelWindowProperty(bool assert1, bool assert2, bool assert3)
  {
    auto levelWindowProperty1 =
      dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode1->GetProperty("levelwindow"));
    auto levelWindowProperty2 =
      dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode2->GetProperty("levelwindow"));
    auto levelWindowProperty3 =
      dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode3->GetProperty("levelwindow"));

    // Check if the active level window property of the manager is equal to any of the level window properties of the nodes
    auto managerLevelWindowProperty = m_LevelWindowManager->GetLevelWindowProperty();

    return (assert1 == (managerLevelWindowProperty == levelWindowProperty1)) &&
           (assert2 == (managerLevelWindowProperty == levelWindowProperty2)) &&
           (assert3 == (managerLevelWindowProperty == levelWindowProperty3));
  }

  bool AssertCurrentNode(const mitk::DataNode *node)
  {
    return m_LevelWindowManager->GetCurrentNode() == node;
  }

  static mitk::LevelWindowProperty *GetLevelWindowProperty(const mitk::DataNode *node)
  {
    return dynamic_cast<mitk::LevelWindowProperty *>(node->GetProperty("levelwindow"));
  }

  mitk::DataNode::Pointer LoadAdditionalNode(int layer)
  {
    auto node = mitk::IOUtil::Load(m_ImagePath1, *m_DataManager)->GetElement(0);
    node->SetIntProperty("layer", layer);
    return node;
  }

public:

  void setUp() override
  {
    m_LevelWindowManager = mitk::LevelWindowManager::New();
    m_DataManager = mitk::StandaloneDataStorage::New();

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("DataStorage could not be set for the new level window manager", m_LevelWindowManager->SetDataStorage(m_DataManager));
    CPPUNIT_ASSERT_MESSAGE("DataStorage could not be retrieved from the new level window manager", m_DataManager == m_LevelWindowManager->GetDataStorage());

    m_ImagePath1 = GetTestDataFilePath("Pic3D.nrrd");
    m_ImagePath2 = GetTestDataFilePath("UltrasoundImages/4D_TEE_Data_MV.dcm");
    m_ImagePath3 = GetTestDataFilePath("RenderingTestData/defaultWatermark.png");

    // add multiple objects to the data storage => property observers will be created
    m_DataNode1 = mitk::IOUtil::Load(m_ImagePath1, *m_DataManager)->GetElement(0);
    m_DataNode2 = mitk::IOUtil::Load(m_ImagePath2, *m_DataManager)->GetElement(0);
    CPPUNIT_ASSERT_MESSAGE("Not two relevant nodes found in the data storage",
      m_LevelWindowManager->GetRelevantNodes()->size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Not two observers created for the relevant nodes",
      m_LevelWindowManager->GetNumberOfObservers() == 2);

    m_DataNode1->SetIntProperty("layer", 1);
    m_DataNode2->SetIntProperty("layer", 2);

    bool isImageForLevelWindow1, isImageForLevelWindow2;
    m_DataNode1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    m_DataNode2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    CPPUNIT_ASSERT_MESSAGE("Initial \"imageForLevelWindow\" property not exclusively set for node 2",
      !isImageForLevelWindow1 && isImageForLevelWindow2);

    m_DataNode3 = mitk::IOUtil::Load(m_ImagePath3, *m_DataManager)->GetElement(0);
    CPPUNIT_ASSERT_MESSAGE("Not three relevant nodes found in the data storage",
      m_LevelWindowManager->GetRelevantNodes()->size() == 3);
    CPPUNIT_ASSERT_MESSAGE("Not three observers created for the relevant nodes",
      m_LevelWindowManager->GetNumberOfObservers() == 3);

    m_DataNode3->SetIntProperty("layer", 3);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("Topmost node is not the current node", AssertCurrentNode(m_DataNode3));
  }

  void tearDown() override {}

  void TestModes()
  {
    CPPUNIT_ASSERT_MESSAGE("TopMostImage is not the default mode", Mode::TopMostImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Apply to all selected images is not disabled per default", !m_LevelWindowManager->GetApplyToAllSelectedImages());

    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    CPPUNIT_ASSERT_MESSAGE("Mode was not switched to SelectedImage", Mode::SelectedImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("No node is selected but there is a current node", AssertCurrentNode(nullptr));
    CPPUNIT_ASSERT_MESSAGE("No node is selected but there is a level window property", m_LevelWindowManager->GetLevelWindowProperty().IsNull());
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));

    m_LevelWindowManager->SetMode(Mode::TopMostImage);
    CPPUNIT_ASSERT_MESSAGE("Mode was not switched to TopMostImage", Mode::TopMostImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Topmost node is not the current node", AssertCurrentNode(m_DataNode3));

    // Switching to ExplicitImage pins the current node
    m_LevelWindowManager->SetMode(Mode::ExplicitImage);
    CPPUNIT_ASSERT_MESSAGE("Mode was not switched to ExplicitImage", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode3));

    m_DataNode1->SetIntProperty("layer", 10);
    CPPUNIT_ASSERT_MESSAGE("Pinned node was not kept after a layer change", AssertCurrentNode(m_DataNode3));

    m_LevelWindowManager->SetMode(Mode::TopMostImage);
    CPPUNIT_ASSERT_MESSAGE("Topmost node is not the current node", AssertCurrentNode(m_DataNode1));
  }

  void TestSetModeWithoutDataStorage()
  {
    auto levelWindowManager = mitk::LevelWindowManager::New();

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("SetMode threw without a data storage", levelWindowManager->SetMode(Mode::SelectedImage));
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("SetApplyToAllSelectedImages threw without a data storage", levelWindowManager->SetApplyToAllSelectedImages(true));
    CPPUNIT_ASSERT_MESSAGE("Mode was not stored", Mode::SelectedImage == levelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Option was not stored", levelWindowManager->GetApplyToAllSelectedImages());
    CPPUNIT_ASSERT_THROW_MESSAGE("SetLevelWindowProperty did not throw without a data storage",
      levelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode1)), mitk::Exception);

    m_DataNode1->SetSelected(true);
    levelWindowManager->SetDataStorage(m_DataManager);
    CPPUNIT_ASSERT_MESSAGE("Mode did not survive SetDataStorage", Mode::SelectedImage == levelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Selected node is not the current node", levelWindowManager->GetCurrentNode() == m_DataNode1);
  }

  void TestSetLevelWindowProperty()
  {
    // Setting the level window property of the manager pins the corresponding node
    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode1));
    CPPUNIT_ASSERT_MESSAGE("Mode was not switched to ExplicitImage", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode1));

    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode2));
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode2));

    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode3));
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode3));

    CPPUNIT_ASSERT_THROW_MESSAGE("Expected exception for an unknown level window property was not thrown",
      m_LevelWindowManager->SetLevelWindowProperty(mitk::LevelWindowProperty::New()), mitk::Exception);
    CPPUNIT_ASSERT_MESSAGE("Failed call changed the current node", AssertCurrentNode(m_DataNode3));
    CPPUNIT_ASSERT_MESSAGE("Failed call changed the mode", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
  }

  void TestVisibilityPropertyChanged()
  {
    // Hiding a node will make the "next" node be the "imageForLevelWindow" node, if
    // the hidden node was the "imageForLevelWindow" node before. "Next" is dependent on the node layer.
    m_DataNode3->SetVisibility(false);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode2->SetVisibility(false);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode1->SetVisibility(false);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());

    m_DataNode3->SetVisibility(true);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));
  }

  void TestLayerPropertyChanged()
  {
    m_DataNode3->SetIntProperty("layer", itk::NumericTraits<int>::min());
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode2->SetIntProperty("layer", itk::NumericTraits<int>::min());
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode1->SetIntProperty("layer", itk::NumericTraits<int>::min());
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());

    m_DataNode3->SetIntProperty("layer", 1);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));
  }

  void TestImageRenderingModePropertyChanged()
  {
    // checking the default rendering mode
    auto renderingMode = dynamic_cast<mitk::RenderingModeProperty*>(m_DataNode3->GetProperty("Image Rendering.Mode"));
    CPPUNIT_ASSERT_MESSAGE("Initial \"Image Rendering.Mode\" property not set to \"RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR\"",
      mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR == renderingMode->GetRenderingMode());

    // Changing the "Image Rendering.Mode" of a node to either "LOOKUPTABLE_COLOR" or
    // "COLORTRANSFERFUNCTION_COLOR" will ignore this node for the level window
    m_DataNode3->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_COLOR));
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode2->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR));
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode1->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_COLOR));
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());

    m_DataNode3->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR));
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));
  }

  void TestSelectedPropertyChanged()
  {
    // In TopMostImage mode the selection is ignored
    m_DataNode1->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));

    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    // Among several selected nodes the one with the highest layer wins
    m_DataNode2->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode3->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));

    m_DataNode1->SetSelected(false);
    m_DataNode2->SetSelected(false);
    m_DataNode3->SetSelected(false);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());
    CPPUNIT_ASSERT_MESSAGE("Current node is not null", AssertCurrentNode(nullptr));
  }

  void TestDeterministicTopMostSelectedNode()
  {
    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    m_DataNode1->SetSelected(true);
    m_DataNode2->SetSelected(true);
    m_DataNode3->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("Topmost selected node is not the current node", AssertCurrentNode(m_DataNode3));

    m_DataNode3->SetIntProperty("layer", itk::NumericTraits<int>::min());
    CPPUNIT_ASSERT_MESSAGE("Topmost selected node is not the current node", AssertCurrentNode(m_DataNode2));

    m_DataNode2->SetSelected(false);
    CPPUNIT_ASSERT_MESSAGE("Topmost selected node is not the current node", AssertCurrentNode(m_DataNode1));

    // An invisible selected node is not a candidate and there is no fallback
    m_DataNode1->SetVisibility(false);
    CPPUNIT_ASSERT_MESSAGE("Current node is not null", AssertCurrentNode(nullptr));
    CPPUNIT_ASSERT_MESSAGE("Mode was changed", Mode::SelectedImage == m_LevelWindowManager->GetMode());

    m_DataNode1->SetVisibility(true);
    CPPUNIT_ASSERT_MESSAGE("Topmost selected node is not the current node", AssertCurrentNode(m_DataNode1));
  }

  void TestApplyToAllSelectedImages()
  {
    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    m_DataNode1->SetSelected(true);
    m_DataNode2->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("Topmost selected node is not the current node", AssertCurrentNode(m_DataNode2));

    const mitk::LevelWindow unchangedLevelWindow = GetLevelWindowProperty(m_DataNode3)->GetLevelWindow();

    m_LevelWindowManager->SetApplyToAllSelectedImages(true);
    const mitk::LevelWindow sharedLevelWindow(100.0, 50.0);
    m_LevelWindowManager->SetLevelWindow(sharedLevelWindow);
    CPPUNIT_ASSERT_MESSAGE("Level window not applied to the current node", GetLevelWindowProperty(m_DataNode2)->GetLevelWindow() == sharedLevelWindow);
    CPPUNIT_ASSERT_MESSAGE("Level window not applied to the other selected node", GetLevelWindowProperty(m_DataNode1)->GetLevelWindow() == sharedLevelWindow);
    CPPUNIT_ASSERT_MESSAGE("Level window applied to an unselected node", GetLevelWindowProperty(m_DataNode3)->GetLevelWindow() == unchangedLevelWindow);

    m_LevelWindowManager->SetApplyToAllSelectedImages(false);
    const mitk::LevelWindow singleLevelWindow(200.0, 80.0);
    m_LevelWindowManager->SetLevelWindow(singleLevelWindow);
    CPPUNIT_ASSERT_MESSAGE("Level window not applied to the current node", GetLevelWindowProperty(m_DataNode2)->GetLevelWindow() == singleLevelWindow);
    CPPUNIT_ASSERT_MESSAGE("Level window applied to the other selected node", GetLevelWindowProperty(m_DataNode1)->GetLevelWindow() == sharedLevelWindow);
  }

  void TestExplicitImageSticks()
  {
    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode1));
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode1));

    m_DataNode2->SetIntProperty("layer", 10);
    CPPUNIT_ASSERT_MESSAGE("Pinned node not kept after a layer change", AssertCurrentNode(m_DataNode1));

    m_DataNode3->SetVisibility(false);
    m_DataNode3->SetVisibility(true);
    CPPUNIT_ASSERT_MESSAGE("Pinned node not kept after a visibility change", AssertCurrentNode(m_DataNode1));

    m_DataNode2->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("Pinned node not kept after a selection change", AssertCurrentNode(m_DataNode1));

    m_DataNode2->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_COLOR));
    m_DataNode2->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));
    CPPUNIT_ASSERT_MESSAGE("Pinned node not kept after a rendering mode change", AssertCurrentNode(m_DataNode1));

    CPPUNIT_ASSERT_MESSAGE("Mode was changed", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));

    m_LevelWindowManager->SetMode(Mode::TopMostImage);
    CPPUNIT_ASSERT_MESSAGE("Topmost node is not the current node", AssertCurrentNode(m_DataNode2));
  }

  void TestExplicitNodeInvisibleThenVisible()
  {
    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode1));

    // While the pinned node is hidden, the topmost visible node is shown but the pin is kept
    m_DataNode1->SetVisibility(false);
    CPPUNIT_ASSERT_MESSAGE("Topmost node is not shown while the pinned node is hidden", AssertCurrentNode(m_DataNode3));
    CPPUNIT_ASSERT_MESSAGE("Mode was changed", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));

    m_DataNode1->SetVisibility(true);
    CPPUNIT_ASSERT_MESSAGE("Pinned node did not come back", AssertCurrentNode(m_DataNode1));

    // The same applies to a pinned node that is not rendered with a level window
    m_DataNode1->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_COLOR));
    CPPUNIT_ASSERT_MESSAGE("Topmost node is not shown while the pinned node is not level-window rendered", AssertCurrentNode(m_DataNode3));

    m_DataNode1->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));
    CPPUNIT_ASSERT_MESSAGE("Pinned node did not come back", AssertCurrentNode(m_DataNode1));
  }

  void TestModeSticksAcrossNodeAdd()
  {
    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    m_DataNode1->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("Selected node is not the current node", AssertCurrentNode(m_DataNode1));

    auto dataNode4 = this->LoadAdditionalNode(4);
    CPPUNIT_ASSERT_MESSAGE("Not four observers created for the relevant nodes", m_LevelWindowManager->GetNumberOfObservers() == 4);
    CPPUNIT_ASSERT_MESSAGE("Mode was reset by adding a node", Mode::SelectedImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Selected node is not the current node", AssertCurrentNode(m_DataNode1));

    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode2));
    auto dataNode5 = this->LoadAdditionalNode(5);
    CPPUNIT_ASSERT_MESSAGE("Mode was reset by adding a node", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode2));

    m_LevelWindowManager->SetMode(Mode::TopMostImage);
    CPPUNIT_ASSERT_MESSAGE("Topmost node is not the current node", AssertCurrentNode(dataNode5));
  }

  void TestModeSticksAcrossSetDataStorage()
  {
    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    m_DataNode1->SetSelected(true);

    m_LevelWindowManager->SetDataStorage(m_DataManager);
    CPPUNIT_ASSERT_MESSAGE("Mode was reset by setting the data storage again", Mode::SelectedImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Selected node is not the current node", AssertCurrentNode(m_DataNode1));
    CPPUNIT_ASSERT_MESSAGE("Observers were duplicated", m_LevelWindowManager->GetNumberOfObservers() == 3);

    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode2));
    m_LevelWindowManager->SetDataStorage(m_DataManager);
    CPPUNIT_ASSERT_MESSAGE("Mode was reset by setting the data storage again", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode2));
  }

  void TestRemoveDataNodes()
  {
    m_DataManager->Remove(m_DataNode3);
    CPPUNIT_ASSERT_MESSAGE("Node not correctly removed", m_LevelWindowManager->GetRelevantNodes()->size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Observer not correctly removed", m_LevelWindowManager->GetNumberOfObservers() == 2);

    bool isImageForLevelWindow1, isImageForLevelWindow2;
    m_DataNode1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    m_DataNode2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", !isImageForLevelWindow1 && isImageForLevelWindow2);

    auto levelWindowProperty1 = GetLevelWindowProperty(m_DataNode1);
    auto levelWindowProperty2 = GetLevelWindowProperty(m_DataNode2);
    auto managerLevelWindowProperty = m_LevelWindowManager->GetLevelWindowProperty();
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set",
      (false == (managerLevelWindowProperty == levelWindowProperty1)) &&
      (true == (managerLevelWindowProperty == levelWindowProperty2)));

    m_DataManager->Remove(m_DataNode2);
    CPPUNIT_ASSERT_MESSAGE("Node not correctly removed", m_LevelWindowManager->GetRelevantNodes()->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Observer not correctly removed", m_LevelWindowManager->GetNumberOfObservers() == 1);

    m_DataNode1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", isImageForLevelWindow1);

    levelWindowProperty1 = GetLevelWindowProperty(m_DataNode1);
    managerLevelWindowProperty = m_LevelWindowManager->GetLevelWindowProperty();
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", true == (managerLevelWindowProperty == levelWindowProperty1));

    m_DataManager->Remove(m_DataNode1);
    CPPUNIT_ASSERT_MESSAGE("Node not correctly removed", m_LevelWindowManager->GetRelevantNodes()->size() == 0);
    CPPUNIT_ASSERT_MESSAGE("Observer not correctly removed", m_LevelWindowManager->GetNumberOfObservers() == 0);

    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());
    CPPUNIT_ASSERT_MESSAGE("Current node is not null", AssertCurrentNode(nullptr));
  }

  void TestRemoveNodeInSelectedImageMode()
  {
    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    m_DataNode1->SetSelected(true);
    m_DataNode2->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("Topmost selected node is not the current node", AssertCurrentNode(m_DataNode2));

    m_DataManager->Remove(m_DataNode2);
    CPPUNIT_ASSERT_MESSAGE("Observer not correctly removed", m_LevelWindowManager->GetNumberOfObservers() == 2);
    CPPUNIT_ASSERT_MESSAGE("Mode was changed by removing the current node", Mode::SelectedImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Remaining selected node is not the current node", AssertCurrentNode(m_DataNode1));

    m_DataManager->Remove(m_DataNode1);
    CPPUNIT_ASSERT_MESSAGE("Mode was changed by removing the current node", Mode::SelectedImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Current node is not null", AssertCurrentNode(nullptr));
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());
  }

  void TestRemoveNodeInExplicitImageMode()
  {
    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode1));

    m_DataManager->Remove(m_DataNode3);
    CPPUNIT_ASSERT_MESSAGE("Mode was changed by removing another node", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode1));

    m_DataManager->Remove(m_DataNode1);
    CPPUNIT_ASSERT_MESSAGE("Mode did not revert to TopMostImage after removing the pinned node", Mode::TopMostImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Topmost node is not the current node", AssertCurrentNode(m_DataNode2));

    // A pick made in SelectedImage mode returns to SelectedImage mode
    auto dataNode4 = this->LoadAdditionalNode(4);
    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    m_DataNode2->SetSelected(true);
    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(dataNode4));
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(dataNode4));

    m_DataManager->Remove(dataNode4);
    CPPUNIT_ASSERT_MESSAGE("Mode did not revert to SelectedImage after removing the pinned node", Mode::SelectedImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Selected node is not the current node", AssertCurrentNode(m_DataNode2));

    // The same applies to a pin made via SetMode
    m_LevelWindowManager->SetMode(Mode::ExplicitImage);
    m_DataNode2->SetSelected(false);
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode2));

    m_DataManager->Remove(m_DataNode2);
    CPPUNIT_ASSERT_MESSAGE("Mode did not revert to SelectedImage after removing the pinned node", Mode::SelectedImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Current node is not null", AssertCurrentNode(nullptr));
  }

  void TestCurrentNodeAndNameChange()
  {
    CPPUNIT_ASSERT_MESSAGE("Current node is wrong", AssertCurrentNode(m_DataNode3));
    CPPUNIT_ASSERT_MESSAGE("Current image is wrong", m_LevelWindowManager->GetCurrentImage() == m_DataNode3->GetData());

    const auto modifiedTime = m_LevelWindowManager->GetMTime();
    m_DataNode3->SetName("renamed");
    CPPUNIT_ASSERT_MESSAGE("Renaming a node did not modify the manager", m_LevelWindowManager->GetMTime() > modifiedTime);

    m_DataNode1->SetVisibility(false);
    m_DataNode2->SetVisibility(false);
    m_DataNode3->SetVisibility(false);
    CPPUNIT_ASSERT_MESSAGE("Current node is not null", AssertCurrentNode(nullptr));
    CPPUNIT_ASSERT_MESSAGE("Current image is not null", nullptr == m_LevelWindowManager->GetCurrentImage());
  }

  void TestCombinedPropertiesChanged()
  {
    m_LevelWindowManager->SetMode(Mode::SelectedImage);
    m_DataNode1->SetSelected(true); // node1 is the only selected node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode2->SetIntProperty("layer", itk::NumericTraits<int>::max()); // node2 is not selected and therefore ignored
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode1->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR)); // node1 is ignored - no current node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));

    m_LevelWindowManager->SetMode(Mode::TopMostImage); // node2 has the highest layer
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode3)); // pin node3
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("Mode was not switched to ExplicitImage", Mode::ExplicitImage == m_LevelWindowManager->GetMode());

    m_DataNode3->SetVisibility(false); // pinned node3 hidden - topmost visible node2 is shown, pin is kept
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("Mode was changed", Mode::ExplicitImage == m_LevelWindowManager->GetMode());

    m_DataNode2->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_COLOR)); // no candidate left: node1 and node2 are not level-window rendered, node3 is hidden
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());

    m_LevelWindowManager->SetMode(Mode::TopMostImage); // still no candidate
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));

    m_LevelWindowManager->SetLevelWindowProperty(GetLevelWindowProperty(m_DataNode3)); // pin the hidden node3 - no candidate for the fallback
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("Mode was not switched to ExplicitImage", Mode::ExplicitImage == m_LevelWindowManager->GetMode());

    m_DataNode1->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR)); // pinned node3 hidden - node1 is the only candidate for the fallback
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode2->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR)); // pinned node3 hidden - node2 is the topmost candidate
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode2->SetIntProperty("layer", itk::NumericTraits<int>::min()); // pinned node3 hidden - node1 is the topmost candidate
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode3->SetVisibility(true); // pinned node3 is visible again
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("Mode was changed", Mode::ExplicitImage == m_LevelWindowManager->GetMode());
    CPPUNIT_ASSERT_MESSAGE("Pinned node is not the current node", AssertCurrentNode(m_DataNode3));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLevelWindowManager)
