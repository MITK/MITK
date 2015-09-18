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

#include "mitkTestFixture.h"
#include "mitkException.h"
#include "mitkTestingMacros.h"

#include "mitkIOUtil.h"
#include "mitkSceneIO.h"
#include "mitkSceneIOTestScenarioProvider.h"
#include "mitkDataStorageCompare.h"

/**
  \brief Test cases for SceneIO.

  Notes on tests for SceneIO (topics to test, details to keep in mind, methods to use):
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
    - failed properties are not meaningful, they lack association with a data node (even if it could be deduced from the input data storage)
    - don't test that too much, this is worth re-thinking
  - I/O tests
    - reproduction of input storage (general):
      - write to a valid path, non-existing archive
      - create a rich input data storage (several cases, see below)
      - read the created archive, compare the output to the original input
        - check if implementing mitk::Equal(DataStorage, DataStorage) is a viable option
        - all nodes must be present, no additional ones
          - data: shall be nullptr as in input or same data type.
            - comparisons of data need to be specific and needs to be extendable for new types
          - geometry of data: shall be equal except for MTimes
          - interactors: will be nullptr! clarify in doc? perhaps clear to everybody..
          - renderer/mapper: will be reset to defaults! clarify? consider in new implementation?
          - properties: all property lists for all render windows shall be re-created
            - for each propertylist, all properties shall be re-created
              - name, type, value
              - smartpointerproperties??
        - the hierachy between nodes shall be re-created correctly
          - multiple parents are possible and shall be tested!
        - data storage could be compared: calculate bounds, get group tags
    - reproduction of specific storage situations:
      - empty storage
      - long flat list
      - long degenerated tree
      - tree with depth of 10 and nodes with multiple parents from:
        - higher hierarchy levels, from sibling trees
        - one higher, one same hierarchy level
        - all other nodes :-)
        - review restrictions of DataStorage, check its limits
      - empty data members
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

  - Tests scenarios and comparisons should be kept out of
    this test for SceneIO because they should be re-used for
    a possible re-write!

*/
class mitkMoreSceneIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMoreSceneIOTestSuite);
  MITK_TEST(Test_SceneIOInterfaces);
  MITK_TEST(Test_ReconstructionOfScenes);
  MITK_TEST(Test_RenderingOfHistoricScenes);
  CPPUNIT_TEST_SUITE_END();

  mitk::SceneIOTestScenarioProvider m_TestCaseProvider;

public:

  void Test_SceneIOInterfaces()
  {
    CPPUNIT_ASSERT_MESSAGE("Not urgent", true);
  }

  void Test_ReconstructionOfScenes()
  {
    std::string tempDir = mitk::IOUtil::CreateTemporaryDirectory("SceneIOTest_XXXXXX");

    mitk::SceneIOTestScenarioProvider::ScenarioList scenarios = m_TestCaseProvider.GetAllScenarios();
    for (auto scenario : scenarios)
    {
      MITK_TEST_OUTPUT(<< "\n===== Test_ReconstructionOfScenes, scenario '" << scenario.key << "' =====");

      std::string archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", tempDir);
      mitk::SceneIO::Pointer writer = mitk::SceneIO::New();
      mitk::DataStorage::Pointer originalStorage = scenario.BuildDataStorage();
      CPPUNIT_ASSERT_MESSAGE(std::string("Save test scenario '") + scenario.key + "' to '" + archiveFilename + "'",
          scenario.serializable == writer->SaveScene(originalStorage->GetAll(), originalStorage, archiveFilename)
          );

      if (scenario.serializable)
      {
        mitk::SceneIO::Pointer reader = mitk::SceneIO::New();
        mitk::DataStorage::Pointer restoredStorage;
        CPPUNIT_ASSERT_NO_THROW(restoredStorage = reader->LoadScene(archiveFilename));
        CPPUNIT_ASSERT_MESSAGE(std::string("Comparing restored test scenario '") + scenario.key + "'",
            mitk::DataStorageCompare(originalStorage,
                                     restoredStorage,
                                     mitk::DataStorageCompare::CMP_Hierarchy
                                     //mitk::DataStorageCompare::CMP_Data |
                                     //mitk::DataStorageCompare::CMP_Properties |
                                     //mitk::DataStorageCompare::CMP_Mappers |
                                     //mitk::DataStorageCompare::CMP_Interactors
                                     // mappers and interactors skipped for now
                                     ).CompareVerbose()
            );
      }
    }
  }

  void Test_RenderingOfHistoricScenes()
  {
    CPPUNIT_ASSERT_MESSAGE("Test elsewhere? Structure of rendering tests could impose that", true);
  }

}; // class

MITK_TEST_SUITE_REGISTRATION(mitkMoreSceneIO)
