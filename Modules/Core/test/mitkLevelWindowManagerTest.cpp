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

#include <itkComposeImageFilter.h>
#include <itkEventObject.h>
#include <itkImageDuplicator.h>
#include <itkImageIterator.h>
#include <mitkImageCast.h>

class mitkLevelWindowManagerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLevelWindowManagerTestSuite);
  MITK_TEST(TestModes);
  MITK_TEST(TestSetLevelWindowProperty);
  MITK_TEST(TestVisibilityPropertyChanged);
  MITK_TEST(TestLayerPropertyChanged);
  MITK_TEST(TestImageRenderingModePropertyChanged);
  MITK_TEST(TestImageForLevelWindowPropertyChanged);
  MITK_TEST(TestSelectedPropertyChanged);
  MITK_TEST(TestRemoveDataNodes);
  MITK_TEST(TestCombinedPropertiesChanged);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::LevelWindowManager::Pointer m_LevelWindowManager;
  mitk::StandaloneDataStorage::Pointer m_DataManager;

  std::string m_ImagePath1;
  std::string m_ImagePath2;
  std::string m_ImagePath3;

  mitk::DataNode::Pointer m_DataNode1;
  mitk::DataNode::Pointer m_DataNode2;
  mitk::DataNode::Pointer m_DataNode3;

  mitk::Image::Pointer m_mitkMultiComponentImage;
  mitk::Image::Pointer m_mitkImageComponent1;
  mitk::Image::Pointer m_mitkImageComponent2;

  bool AssertImageForLevelWindowProperty(bool assert1, bool assert2, bool assert3)
  {
    bool imageForLevelWindowProperty1, imageForLevelWindowProperty2, imageForLevelWindowProperty3;

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
    AssertImageForLevelWindowProperty(false, false, true);
  }

  void tearDown() override {}

  void TestModes()
  {
    CPPUNIT_ASSERT_MESSAGE("AutoTopMost mode is not enabled per default", m_LevelWindowManager->IsAutoTopMost());
    CPPUNIT_ASSERT_MESSAGE("SelectedImagesMode mode is not disabled per default", !m_LevelWindowManager->IsSelectedImages());

    m_LevelWindowManager->SetSelectedImages(true);
    CPPUNIT_ASSERT_MESSAGE("AutoTopMost mode was not disabled on mode switch", !m_LevelWindowManager->IsAutoTopMost());
    CPPUNIT_ASSERT_MESSAGE("SelectedImagesMode mode was not enabled on mode switch", m_LevelWindowManager->IsSelectedImages());

    m_LevelWindowManager->SetSelectedImages(false);
    CPPUNIT_ASSERT_MESSAGE("AutoTopMost mode was not disabled on mode switch", !m_LevelWindowManager->IsAutoTopMost());
    CPPUNIT_ASSERT_MESSAGE("SelectedImagesMode mode was not disabled on mode switch", !m_LevelWindowManager->IsSelectedImages());

    m_LevelWindowManager->SetSelectedImages(true); // to enable "SelectedImagesMode"
    m_LevelWindowManager->SetAutoTopMostImage(true);
    CPPUNIT_ASSERT_MESSAGE("AutoTopMost mode was not enabled on mode switch", m_LevelWindowManager->IsAutoTopMost());
    CPPUNIT_ASSERT_MESSAGE("SelectedImagesMode mode was not disabled on mode switch", !m_LevelWindowManager->IsSelectedImages());
  }

  void TestSetLevelWindowProperty()
  {
    m_LevelWindowManager->SetAutoTopMostImage(false);

    // Setting the level window property of the manager
    // will make the corresponding node be the "imageForLevelWindow" node.
    auto levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode1->GetProperty("levelwindow"));
    m_LevelWindowManager->SetLevelWindowProperty(levelWindowProperty);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode2->GetProperty("levelwindow"));
    m_LevelWindowManager->SetLevelWindowProperty(levelWindowProperty);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode3->GetProperty("levelwindow"));
    m_LevelWindowManager->SetLevelWindowProperty(levelWindowProperty);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));

    levelWindowProperty = mitk::LevelWindowProperty::New();
    //CPPUNIT_ASSERT_THROW_MESSAGE("Expected exception for an invalid level window property was not thrown",
      //m_LevelWindowManager->SetLevelWindowProperty(levelWindowProperty), mitk::Exception);
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

  void TestImageForLevelWindowPropertyChanged()
  {
    m_LevelWindowManager->SetAutoTopMostImage(false);

    m_DataNode1->SetBoolProperty("imageForLevelWindow", true);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode2->SetBoolProperty("imageForLevelWindow", true);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode3->SetBoolProperty("imageForLevelWindow", true);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));

    // The top level node will alsways be used as a fall back node
    // if no specific mode is selected.
    m_DataNode3->SetBoolProperty("imageForLevelWindow", false);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));
  }

  void TestSelectedPropertyChanged()
  {
    // Selecting a node will make this node be the "imageForLevelWindow" node, if
    // the "SelectedImagesMode" is enabled (disabled per default).
    m_DataNode1->SetSelected(true); // selection mode not enabled - node3 stays the "imageForLevelWindow" node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));

    // This will immediately check for the selected node (node 1).
    m_LevelWindowManager->SetSelectedImages(true);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    // Second node will be selected (node 1 and node 2 selected).
    // It is not specified which data node will be used for the "imageForLevelWindow" / "levelwindow" property,
    // since the data storage access to the nodes is non-deterministic.
    // At least check for any valid level window property.
    m_DataNode2->SetSelected(true);
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is null", m_LevelWindowManager->GetLevelWindowProperty());

    // Third node will be selected (node 1, node 2 and node 3 selected)
    // It is not specified which data node will be used for the "imageForLevelWindow" / "levelwindow" property,
    // since the data storage access to the nodes is non-deterministic.
    // At least check for any valid level window property.
    m_DataNode3->SetSelected(true);
    auto usedLevelWindowProperty = m_LevelWindowManager->GetLevelWindowProperty();
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is null", usedLevelWindowProperty);

    // All three nodes are selected: Check if only one node has the "imageForLevelWindow" property set and this node's
    // "levelwindow" property is used by the level window manager.
    auto levelWindowProperty1 = dynamic_cast<mitk::LevelWindowProperty*>(m_DataNode1->GetProperty("imageForLevelWindow"));
    if (usedLevelWindowProperty == levelWindowProperty1)
    {
      CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
      CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));
    }

    auto levelWindowProperty2 = dynamic_cast<mitk::LevelWindowProperty*>(m_DataNode2->GetProperty("imageForLevelWindow"));
    if (usedLevelWindowProperty == levelWindowProperty2)
    {
      CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
      CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));
    }

    auto levelWindowProperty3 = dynamic_cast<mitk::LevelWindowProperty*>(m_DataNode3->GetProperty("imageForLevelWindow"));
    if (usedLevelWindowProperty == levelWindowProperty3)
    {
      CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
      CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));
    }

    m_DataNode1->SetSelected(false);
    m_DataNode2->SetSelected(false);
    m_DataNode3->SetSelected(false);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());
  }

  void TestCombinedPropertiesChanged()
  {
    m_LevelWindowManager->SetSelectedImages(true);
    m_DataNode1->SetSelected(true); // selection mode enabled - node1 becomes the "imageForLevelWindow" node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode2->SetIntProperty("layer", itk::NumericTraits<int>::max()); // selection mode enabled - node1 stays the "imageForLevelWindow" node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode1->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR)); // selection mode enabled - but node1 is ignored - no active level window property
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));

    m_DataNode3->SetBoolProperty("imageForLevelWindow", true); // selection mode enabled - no valid node selected, no active level window property
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));

    m_LevelWindowManager->SetSelectedImages(false); // no mode enabled - however, level window is not modified / updated
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));

    m_LevelWindowManager->Update(itk::ModifiedEvent()); // no mode enabled - level window is modified / updated with topmost visible node2 as fallback
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode3->SetBoolProperty("imageForLevelWindow", true); // no mode enabled - use node3 with "imageForLevelWindow" property set
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));

    m_DataNode3->SetVisibility(false); // node3 with "imageForLevelWindow" property not visible - use topmost visible node2 as fallback
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode2->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_COLOR)); // fallback node2 is ignored - but "imageForLevelWindow" and "levelWindow" stay "true" for node2
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_LevelWindowManager->SetAutoTopMostImage(true); // no visible node exists that is not ignored
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, false));

    auto levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode3->GetProperty("levelwindow"));
    m_LevelWindowManager->SetLevelWindowProperty(levelWindowProperty); // explicitly set the level window to node3
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));

    m_DataNode1->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR)); // auto topmost mode enabled - node1 is the only visible non-ignored node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode2->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(
      mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR)); // auto topmost mode enabled - node2 is topmost visible node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, true, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, true, false));

    m_DataNode2->SetIntProperty("layer", itk::NumericTraits<int>::min()); // auto topmost mode enabled - node1 is topmost visible node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(true, false, false));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(true, false, false));

    m_DataNode3->SetVisibility(true); // auto topmost mode enabled - node3 is topmost visible node
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", AssertImageForLevelWindowProperty(false, false, true));
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", AssertLevelWindowProperty(false, false, true));

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

    auto levelWindowProperty1 = dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode1->GetProperty("levelwindow"));
    auto levelWindowProperty2 = dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode2->GetProperty("levelwindow"));
    auto managerLevelWindowProperty = m_LevelWindowManager->GetLevelWindowProperty();
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set",
      (false == (managerLevelWindowProperty == levelWindowProperty1)) &&
      (true == (managerLevelWindowProperty == levelWindowProperty2)));

    m_DataManager->Remove(m_DataNode2);
    CPPUNIT_ASSERT_MESSAGE("Node not correctly removed", m_LevelWindowManager->GetRelevantNodes()->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Observer not correctly removed", m_LevelWindowManager->GetNumberOfObservers() == 1);

    m_DataNode1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    CPPUNIT_ASSERT_MESSAGE("\"imageForLevelWindow\" property not correctly set", isImageForLevelWindow1);

    levelWindowProperty1 = dynamic_cast<mitk::LevelWindowProperty *>(m_DataNode1->GetProperty("levelwindow"));
    managerLevelWindowProperty = m_LevelWindowManager->GetLevelWindowProperty();
    CPPUNIT_ASSERT_MESSAGE("\"levelwindow\" property not correctly set", true == (managerLevelWindowProperty == levelWindowProperty1));

    m_DataManager->Remove(m_DataNode1);
    CPPUNIT_ASSERT_MESSAGE("Node not correctly removed", m_LevelWindowManager->GetRelevantNodes()->size() == 0);
    CPPUNIT_ASSERT_MESSAGE("Observer not correctly removed", m_LevelWindowManager->GetNumberOfObservers() == 0);

    CPPUNIT_ASSERT_MESSAGE("LevelWindowProperty is not null", !m_LevelWindowManager->GetLevelWindowProperty());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLevelWindowManager)
