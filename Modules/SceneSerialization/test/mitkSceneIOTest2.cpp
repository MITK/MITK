/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkException.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkDataStorageCompare.h"
#include "mitkIOUtil.h"
#include "mitkSceneIO.h"
#include "mitkSceneIOTestScenarioProvider.h"

/**
  \brief Test cases for SceneIO.

  Tests for SceneIO.

  Currently contains:
  - I/O tests: write some Storage, read it, compare

  Should be enhanced with:
  - tests about the interface of SceneIO, providing invalid arguments etc.
  - testing the error reporting (needs rework, however)

  Original notes on tests for SceneIO (topics to test, details to keep in mind, methods to use):
  (this list is a notepad that should be emptied while completing tests)
  - SaveScene interface topics:
    - all input nodes must be in storage
    - no nullptrs (in input parameters, nullptrs as nodes or data can be ok)
    - filename must be something valid
    - empty file shall be written for empty storage (?)
    - failures shall result in return false
  - LoadScene interface topics:
    - filename must exist
    - nullptr storage must be ok
    - when storage is given, it shall be returned
    - clearStorageFirst flag shall be respected
    - existing error reporting should mention failures (create faulty test scene files!)
  - Error reporting methods
    - why do they return C pointers??
    - failed properties are not meaningful, they lack association with a data node (even if it could be deduced from the
  input data storage)
    - don't test that too much, this is worth re-thinking
  - I/O tests:
    - data storage could be compared: calculate bounds, get group tags
    - reproduction of specific storage situations:
      - very long property names (empty ones possible?)
      - empty or very long node names

  - "historic situations", regression tests, bugs that have been discovered and fixed,
    "compatibility tess"
    - re-definition of properties to define lookup-tables in image mapper
      (gray images turn up in all colors of the rainbow)
    - re-definition of size properties from int to float in point or surface mapper
      (sizes changed)
    - such tests could be implemented as rendering tests:
      - create reference scenario manually once, store scene file and expected image in MITK-Data
        - keep data small(!)
        - describe render setups (2D cuts, 3D perspectives)? better standardize one 2D, one 3D view
      - load scene in test, render re-loaded scene, compare

  - (ok) Tests scenarios and comparisons should be kept out of
         this test for SceneIO because they should be re-used for
         a possible re-write!

*/
class mitkSceneIOTest2Suite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSceneIOTest2Suite);
  MITK_TEST(Test_SceneIOInterfaces);
  MITK_TEST(Test_ReconstructionOfScenes);
  CPPUNIT_TEST_SUITE_END();

  mitk::SceneIOTestScenarioProvider m_TestCaseProvider;

public:
  void Test_SceneIOInterfaces() { CPPUNIT_ASSERT_MESSAGE("Not urgent", true); }
  void Test_ReconstructionOfScenes()
  {
    std::string tempDir = mitk::IOUtil::CreateTemporaryDirectory("SceneIOTest_XXXXXX");

    mitk::SceneIOTestScenarioProvider::ScenarioList scenarios = m_TestCaseProvider.GetAllScenarios();
    MITK_TEST_OUTPUT(<< "Executing " << scenarios.size() << " test scenarios");
    for (const auto& scenario : scenarios)
    {
      MITK_TEST_OUTPUT(<< "\n===== Test_ReconstructionOfScenes, scenario '" << scenario.key << "' =====");

      std::string archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", tempDir);
      mitk::SceneIO::Pointer writer = mitk::SceneIO::New();
      mitk::DataStorage::Pointer originalStorage = scenario.BuildDataStorage();
      CPPUNIT_ASSERT_MESSAGE(
        std::string("Save test scenario '") + scenario.key + "' to '" + archiveFilename + "'",
        scenario.serializable == writer->SaveScene(originalStorage->GetAll(), originalStorage, archiveFilename));

      if (scenario.serializable)
      {
        mitk::SceneIO::Pointer reader = mitk::SceneIO::New();
        mitk::DataStorage::Pointer restoredStorage;
        CPPUNIT_ASSERT_NO_THROW(restoredStorage = reader->LoadScene(archiveFilename));
        CPPUNIT_ASSERT_MESSAGE(
          std::string("Comparing restored test scenario '") + scenario.key + "'",
          mitk::DataStorageCompare(originalStorage,
                                   restoredStorage,
                                   // TODO make those flags part of the scenario object
                                   // TODO make known/expected failures also part of the
                                   //      scenario object (but this needs a way to describe them, first)
                                   mitk::DataStorageCompare::CMP_Hierarchy | mitk::DataStorageCompare::CMP_Data |
                                     mitk::DataStorageCompare::CMP_Properties |
                                     // mappers tested although SceneIO leaves default mappers
                                     mitk::DataStorageCompare::CMP_Mappers,
                                   // mitk::DataStorageCompare::CMP_Interactors
                                   // interactors skipped for now
                                   scenario.comparisonPrecision)
            .CompareVerbose());
      }
    }
  }

}; // class

int mitkSceneIOTest2(int /*argc*/, char * /*argv*/ [])
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(mitkSceneIOTest2Suite::suite());
  return runner.run() ? 0 : 1;
}
