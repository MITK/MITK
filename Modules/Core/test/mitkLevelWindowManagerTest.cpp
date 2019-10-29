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

#include "itkMersenneTwisterRandomVariateGenerator.h"
#include "mitkLevelWindowManager.h"
#include "mitkRenderingModeProperty.h"
#include "mitkStandaloneDataStorage.h"
#include <itkComposeImageFilter.h>
#include <itkEventObject.h>
#include <itkImageDuplicator.h>
#include <itkImageIterator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <mitkIOUtil.h>
#include <mitkImageCast.h>
#include <mitkTestingMacros.h>

class mitkLevelWindowManagerTestClass
{
public:
  static void TestInstantiation()
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    MITK_TEST_CONDITION_REQUIRED(manager.IsNotNull(), "Testing mitk::LevelWindowManager::New()");
  }

  static void TestSetGetDataStorage()
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    MITK_TEST_OUTPUT(<< "Creating DataStorage: ");
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

    bool success = true;
    try
    {
      manager->SetDataStorage(ds);
    }
    catch (std::exception &e)
    {
      success = false;
      MITK_ERROR << "Exception: " << e.what();
    }
    MITK_TEST_CONDITION_REQUIRED(success, "Testing mitk::LevelWindowManager SetDataStorage() ");
    MITK_TEST_CONDITION_REQUIRED(ds == manager->GetDataStorage(), "Testing mitk::LevelWindowManager GetDataStorage ");
  }

  static void TestMethodsWithInvalidParameters()
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    bool success = false;
    mitk::LevelWindowProperty::Pointer levelWindowProperty = mitk::LevelWindowProperty::New();
    try
    {
      manager->SetLevelWindowProperty(levelWindowProperty);
    }
    catch (const mitk::Exception &)
    {
      success = true;
    }
    MITK_TEST_CONDITION(success, "Testing mitk::LevelWindowManager SetLevelWindowProperty with invalid parameter");
  }

  static void TestOtherMethods()
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    MITK_TEST_CONDITION(manager->IsAutoTopMost(), "Testing mitk::LevelWindowManager isAutoTopMost");

    // It is not clear what the following code is supposed to test. The expression in
    // the catch(...) block does have no effect, so success is always true.
    // Related bugs are 13894 and 13889
    /*
  bool success = true;
  try
  {
    mitk::LevelWindow levelWindow = manager->GetLevelWindow();
    manager->SetLevelWindow(levelWindow);
  }
  catch (...)
  {
    success == false;
  }
  MITK_TEST_CONDITION(success,"Testing mitk::LevelWindowManager GetLevelWindow() and SetLevelWindow()");
     */

    manager->SetAutoTopMostImage(true);
    MITK_TEST_CONDITION(manager->IsAutoTopMost(), "Testing mitk::LevelWindowManager isAutoTopMost()");
  }

  static void TestRemoveObserver(std::string testImageFile)
  {
    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    // add multiple objects to the data storage => multiple observers should be created
    mitk::Image::Pointer image1 = mitk::IOUtil::Load<mitk::Image>(testImageFile);
    mitk::DataNode::Pointer node1 = mitk::DataNode::New();
    node1->SetData(image1);
    mitk::Image::Pointer image2 = mitk::IOUtil::Load<mitk::Image>(testImageFile);
    mitk::DataNode::Pointer node2 = mitk::DataNode::New();
    node2->SetData(image2);
    ds->Add(node1);
    ds->Add(node2);

    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 2, "Test if nodes have been added");
    MITK_TEST_CONDITION_REQUIRED(
      static_cast<int>(manager->GetRelevantNodes()->size()) == manager->GetNumberOfObservers(),
      "Test if number of nodes is similar to number of observers");

    mitk::Image::Pointer image3 = mitk::IOUtil::Load<mitk::Image>(testImageFile);
    mitk::DataNode::Pointer node3 = mitk::DataNode::New();
    node3->SetData(image3);
    ds->Add(node3);
    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 3, "Test if another node have been added");
    MITK_TEST_CONDITION_REQUIRED(
      static_cast<int>(manager->GetRelevantNodes()->size()) == manager->GetNumberOfObservers(),
      "Test if number of nodes is similar to number of observers");

    ds->Remove(node1);
    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 2, "Deleted node 1 (test GetRelevantNodes())");
    MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 2, "Deleted node 1 (test GetNumberOfObservers())");

    ds->Remove(node2);
    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 1, "Deleted node 2 (test GetRelevantNodes())");
    MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 1, "Deleted node 2 (test GetNumberOfObservers())");

    ds->Remove(node3);
    MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 0, "Deleted node 3 (test GetRelevantNodes())");
    MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 0, "Deleted node 3 (test GetNumberOfObservers())");
  }

  static bool VerifyRenderingModes()
  {
    bool ok = (mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR == 1) &&
              (mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR == 2) &&
              (mitk::RenderingModeProperty::LOOKUPTABLE_COLOR == 3) &&
              (mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR == 4);

    return ok;
  }

  static void TestLevelWindowSliderVisibility(std::string testImageFile)
  {
    bool renderingModesValid = mitkLevelWindowManagerTestClass::VerifyRenderingModes();
    if (!renderingModesValid)
    {
      MITK_ERROR << "Exception: Image Rendering.Mode property value types inconsistent.";
    }

    mitk::LevelWindowManager::Pointer manager;
    manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    // add multiple objects to the data storage => multiple observers should be created
    mitk::Image::Pointer image1 = mitk::IOUtil::Load<mitk::Image>(testImageFile);
    mitk::DataNode::Pointer node1 = mitk::DataNode::New();
    node1->SetData(image1);
    ds->Add(node1);

    mitk::DataNode::Pointer node2 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node3 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    std::vector<mitk::DataNode::Pointer> nodeVec;
    // nodeVec.resize( 3 );
    nodeVec.push_back(node1);
    nodeVec.push_back(node2);
    nodeVec.push_back(node3);

    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator RandomGeneratorType;
    RandomGeneratorType::Pointer rnd = RandomGeneratorType::New();
    rnd->Initialize();

    for (unsigned int i = 0; i < 8; ++i)
    {
      unsigned int parity = i;

      for (unsigned int img = 0; img < 3; ++img)
      {
        if (parity & 1)
        {
          int mode = rnd->GetIntegerVariate() % 3;
          nodeVec[img]->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(mode));
        }
        else
        {
          int mode = rnd->GetIntegerVariate() % 2;
          nodeVec[img]->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(3 + mode));
        }
        parity >>= 1;
      }

      MITK_TEST_CONDITION(
        renderingModesValid && ((!manager->GetLevelWindowProperty() && !i) || (manager->GetLevelWindowProperty() && i)),
        "Testing level window property member according to rendering mode");
    }
  }

  static void TestSetLevelWindowProperty(std::string testImageFile)
  {
    mitk::LevelWindowManager::Pointer manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    // add multiple objects to the data storage => multiple observers should be created
    mitk::DataNode::Pointer node3 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node2 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node1 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);

    node3->SetIntProperty("layer", 1);
    node2->SetIntProperty("layer", 2);
    node1->SetIntProperty("layer", 3);

    manager->SetAutoTopMostImage(true);

    bool isImageForLevelWindow1, isImageForLevelWindow2, isImageForLevelWindow3;
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(isImageForLevelWindow1 && !isImageForLevelWindow2 && !isImageForLevelWindow3,
                        "Testing exclusive imageForLevelWindow property for node 1.");

    manager->SetAutoTopMostImage(false);

    mitk::LevelWindowProperty::Pointer prop =
      dynamic_cast<mitk::LevelWindowProperty *>(node2->GetProperty("levelwindow"));
    manager->SetLevelWindowProperty(prop);
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(!isImageForLevelWindow1 && isImageForLevelWindow2 && !isImageForLevelWindow3,
                        "Testing exclusive imageForLevelWindow property for node 2.");

    prop = dynamic_cast<mitk::LevelWindowProperty *>(node3->GetProperty("levelwindow"));
    manager->SetLevelWindowProperty(prop);
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(!isImageForLevelWindow1 && !isImageForLevelWindow2 && isImageForLevelWindow3,
                        "Testing exclusive imageForLevelWindow property for node 3.");

    prop = dynamic_cast<mitk::LevelWindowProperty *>(node1->GetProperty("levelwindow"));
    manager->SetLevelWindowProperty(prop);
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(isImageForLevelWindow1 && !isImageForLevelWindow2 && !isImageForLevelWindow3,
                        "Testing exclusive imageForLevelWindow property for node 3.");
  }

  static void TestImageForLevelWindowOnVisibilityChange(std::string testImageFile)
  {
    mitk::LevelWindowManager::Pointer manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    // add multiple objects to the data storage => multiple observers should be created
    mitk::DataNode::Pointer node3 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node2 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node1 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);

    node3->SetIntProperty("layer", 1);
    node2->SetIntProperty("layer", 2);
    node1->SetIntProperty("layer", 3);

    manager->SetAutoTopMostImage(false);

    bool isImageForLevelWindow1, isImageForLevelWindow2, isImageForLevelWindow3;
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(isImageForLevelWindow1 && !isImageForLevelWindow2 && !isImageForLevelWindow3,
                        "Testing initial imageForLevelWindow setting.");

    node1->SetVisibility(false);
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(!isImageForLevelWindow1 && isImageForLevelWindow2 && !isImageForLevelWindow3,
                        "Testing exclusive imageForLevelWindow property for node 2.");

    node2->SetVisibility(false);
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(!isImageForLevelWindow1 && !isImageForLevelWindow2 && isImageForLevelWindow3,
                        "Testing exclusive imageForLevelWindow property for node 3.");

    node3->SetVisibility(false);
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(!isImageForLevelWindow1 && !isImageForLevelWindow2 && isImageForLevelWindow3,
                        "Testing exclusive imageForLevelWindow property for node 3.");

    node1->SetVisibility(true);
    node1->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow1);
    node2->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow2);
    node3->GetBoolProperty("imageForLevelWindow", isImageForLevelWindow3);

    MITK_TEST_CONDITION(isImageForLevelWindow1 && !isImageForLevelWindow2 && !isImageForLevelWindow3,
                        "Testing exclusive imageForLevelWindow property for node 3.");
  }

  static void TestImageForLevelWindowOnRandomPropertyChange(std::string testImageFile)
  {
    typedef std::vector<bool> BoolVecType;
    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator RandomGeneratorType;

    // initialize the data storage
    mitk::LevelWindowManager::Pointer manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    mitk::DataNode::Pointer node3 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node2 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node1 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);

    node3->SetIntProperty("layer", 1);
    node2->SetIntProperty("layer", 2);
    node1->SetIntProperty("layer", 3);

    // node visibilities
    std::vector<bool> nodesVisible;
    nodesVisible.resize(3);
    std::fill(nodesVisible.begin(), nodesVisible.end(), true);

    // which node has the level window
    std::vector<bool> nodesForLevelWindow;
    nodesForLevelWindow.resize(3);
    std::fill(nodesForLevelWindow.begin(), nodesForLevelWindow.end(), false);

    // the nodes themselves
    std::vector<mitk::DataNode::Pointer> nodes;
    nodes.push_back(node1);
    nodes.push_back(node2);
    nodes.push_back(node3);

    // status quo
    manager->SetAutoTopMostImage(false);

    bool lvlWin1, lvlWin2, lvlWin3;
    node1->GetBoolProperty("imageForLevelWindow", lvlWin1);
    node2->GetBoolProperty("imageForLevelWindow", lvlWin2);
    node3->GetBoolProperty("imageForLevelWindow", lvlWin3);

    MITK_TEST_CONDITION(lvlWin1 && !lvlWin2 && !lvlWin3, "Testing initial imageForLevelWindow setting.");

    nodesForLevelWindow[0] = lvlWin1;
    nodesForLevelWindow[1] = lvlWin2;
    nodesForLevelWindow[2] = lvlWin3;

    // prepare randomized visibility changes
    RandomGeneratorType::Pointer ranGen = RandomGeneratorType::New();
    ranGen->Initialize();

    int ranCount = 100;
    int validCount = 0;
    int invalidCount = 0;
    int mustHaveLvlWindow = 4;
    for (int run = 0; run < ranCount; ++run)
    {
      // toggle node visibility
      int ran = ranGen->GetIntegerVariate(2);
      nodes[ran]->SetBoolProperty("imageForLevelWindow", !nodesForLevelWindow[ran]);

      // one node must have the level window
      std::vector<bool>::const_iterator found = std::find(nodesForLevelWindow.begin(), nodesForLevelWindow.end(), true);
      if (found == nodesForLevelWindow.end())
      {
        break;
      }

      // all invisible?
      found = std::find(nodesVisible.begin(), nodesVisible.end(), true);

      if (!nodesForLevelWindow[ran])
      {
        mustHaveLvlWindow = pow(2, 2 - ran);
      }
      else
      {
        mustHaveLvlWindow = 4;
      }

      // get the current status
      node1->GetBoolProperty("imageForLevelWindow", lvlWin1);
      node2->GetBoolProperty("imageForLevelWindow", lvlWin2);
      node3->GetBoolProperty("imageForLevelWindow", lvlWin3);
      nodesForLevelWindow[0] = lvlWin1;
      nodesForLevelWindow[1] = lvlWin2;
      nodesForLevelWindow[2] = lvlWin3;

      int hasLevelWindow = 0;
      for (int i = 0; i < 3; ++i)
      {
        if (nodesForLevelWindow[i])
        {
          hasLevelWindow += pow(2, 2 - i);
        }
      }

      validCount += hasLevelWindow == mustHaveLvlWindow ? 1 : 0;

      // test sensitivity
      int falseran = 0;
      while (falseran == 0)
      {
        falseran = ranGen->GetIntegerVariate(7);
      }
      BoolVecType falseNodes;
      falseNodes.push_back((falseran & 1) == 1 ? !lvlWin1 : lvlWin1);
      falseran >>= 1;
      falseNodes.push_back((falseran & 1) == 1 ? !lvlWin2 : lvlWin2);
      falseran >>= 1;
      falseNodes.push_back((falseran & 1) == 1 ? !lvlWin3 : lvlWin3);
      int falseLevelWindow = 0;
      for (int i = 0; i < 3; ++i)
      {
        if (falseNodes[i])
        {
          falseLevelWindow += pow(2, 2 - i);
        }
      }

      invalidCount += falseLevelWindow == mustHaveLvlWindow ? 0 : 1;

      // in case of errors proceed anyway
      mustHaveLvlWindow = hasLevelWindow;
    }

    MITK_TEST_CONDITION(validCount == ranCount, "Testing proper node for level window property.");
    MITK_TEST_CONDITION(invalidCount == ranCount, "Sensitivity test.");
  }

  static void TestImageForLevelWindowOnRandomVisibilityChange(std::string testImageFile)
  {
    typedef std::vector<bool> BoolVecType;
    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator RandomGeneratorType;

    // initialize the data storage
    mitk::LevelWindowManager::Pointer manager = mitk::LevelWindowManager::New();
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    manager->SetDataStorage(ds);

    mitk::DataNode::Pointer node3 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node2 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);
    mitk::DataNode::Pointer node1 = mitk::IOUtil::Load(testImageFile, *ds)->GetElement(0);

    node3->SetIntProperty("layer", 1);
    node2->SetIntProperty("layer", 2);
    node1->SetIntProperty("layer", 3);

    // node visibilities
    std::vector<bool> nodesVisible;
    nodesVisible.resize(3);
    std::fill(nodesVisible.begin(), nodesVisible.end(), true);

    // which node has the level window
    std::vector<bool> nodesForLevelWindow;
    nodesForLevelWindow.resize(3);
    std::fill(nodesForLevelWindow.begin(), nodesForLevelWindow.end(), false);

    // the nodes themselves
    std::vector<mitk::DataNode::Pointer> nodes;
    nodes.push_back(node1);
    nodes.push_back(node2);
    nodes.push_back(node3);

    // status quo
    manager->SetAutoTopMostImage(false);

    bool lvlWin1, lvlWin2, lvlWin3;
    node1->GetBoolProperty("imageForLevelWindow", lvlWin1);
    node2->GetBoolProperty("imageForLevelWindow", lvlWin2);
    node3->GetBoolProperty("imageForLevelWindow", lvlWin3);

    MITK_TEST_CONDITION(lvlWin1 && !lvlWin2 && !lvlWin3, "Testing initial imageForLevelWindow setting.");

    nodesForLevelWindow[0] = lvlWin1;
    nodesForLevelWindow[1] = lvlWin2;
    nodesForLevelWindow[2] = lvlWin3;

    // prepare randomized visibility changes
    RandomGeneratorType::Pointer ranGen = RandomGeneratorType::New();
    ranGen->Initialize();

    int ranCount = 100;
    int validCount = 0;
    int invalidCount = 0;
    int mustHaveLvlWindow = 4;
    for (int run = 0; run < ranCount; ++run)
    {
      // toggle node visibility
      int ran = ranGen->GetIntegerVariate(2);
      nodesVisible[ran] = !nodesVisible[ran];
      nodes[ran]->SetVisibility(nodesVisible[ran]);

      // one node must have the level window
      std::vector<bool>::const_iterator found = std::find(nodesForLevelWindow.begin(), nodesForLevelWindow.end(), true);
      if (found == nodesForLevelWindow.end())
      {
        break;
      }
      int ind = found - nodesForLevelWindow.begin();

      // all invisible?
      found = std::find(nodesVisible.begin(), nodesVisible.end(), true);
      bool allInvisible = (found == nodesVisible.end());

      // which node shall get the level window now
      if (!allInvisible && !nodesVisible[ind])
      {
        int count = 0;
        for (std::vector<bool>::const_iterator it = nodesVisible.begin(); it != nodesVisible.end(); ++it, ++count)
        {
          if (*it)
          {
            mustHaveLvlWindow = pow(2, 2 - count);
            break;
          }
        }
      }

      // get the current status
      node1->GetBoolProperty("imageForLevelWindow", lvlWin1);
      node2->GetBoolProperty("imageForLevelWindow", lvlWin2);
      node3->GetBoolProperty("imageForLevelWindow", lvlWin3);
      nodesForLevelWindow[0] = lvlWin1;
      nodesForLevelWindow[1] = lvlWin2;
      nodesForLevelWindow[2] = lvlWin3;

      int hasLevelWindow = 0;
      for (int i = 0; i < 3; ++i)
      {
        if (nodesForLevelWindow[i])
        {
          hasLevelWindow += pow(2, 2 - i);
        }
      }

      validCount += hasLevelWindow == mustHaveLvlWindow ? 1 : 0;

      // test sensitivity
      int falseran = 0;
      while (falseran == 0)
      {
        falseran = ranGen->GetIntegerVariate(7);
      }
      BoolVecType falseNodes;
      falseNodes.push_back((falseran & 1) == 1 ? !lvlWin1 : lvlWin1);
      falseran >>= 1;
      falseNodes.push_back((falseran & 1) == 1 ? !lvlWin2 : lvlWin2);
      falseran >>= 1;
      falseNodes.push_back((falseran & 1) == 1 ? !lvlWin3 : lvlWin3);
      int falseLevelWindow = 0;
      for (int i = 0; i < 3; ++i)
      {
        if (falseNodes[i])
        {
          falseLevelWindow += pow(2, 2 - i);
        }
      }

      invalidCount += falseLevelWindow == mustHaveLvlWindow ? 0 : 1;

      // in case of errors proceed anyway
      mustHaveLvlWindow = hasLevelWindow;
    }

    MITK_TEST_CONDITION(validCount == ranCount, "Testing proper node for level window property.");
    MITK_TEST_CONDITION(invalidCount == ranCount, "Sensitivity test.");
  }
};

int mitkLevelWindowManagerTest(int argc, char *args[])
{
  MITK_TEST_BEGIN("mitkLevelWindowManager");

  MITK_TEST_CONDITION_REQUIRED(argc >= 2, "Testing if test file is given.");
  std::string testImage = args[1];

  mitkLevelWindowManagerTestClass::TestInstantiation();
  mitkLevelWindowManagerTestClass::TestSetGetDataStorage();
  mitkLevelWindowManagerTestClass::TestMethodsWithInvalidParameters();
  mitkLevelWindowManagerTestClass::TestOtherMethods();
  mitkLevelWindowManagerTestClass::TestRemoveObserver(testImage);
  mitkLevelWindowManagerTestClass::TestLevelWindowSliderVisibility(testImage);
  mitkLevelWindowManagerTestClass::TestSetLevelWindowProperty(testImage);
  mitkLevelWindowManagerTestClass::TestImageForLevelWindowOnVisibilityChange(testImage);
  mitkLevelWindowManagerTestClass::TestImageForLevelWindowOnRandomVisibilityChange(testImage);
  mitkLevelWindowManagerTestClass::TestImageForLevelWindowOnRandomPropertyChange(testImage);

  MITK_TEST_END();
}
