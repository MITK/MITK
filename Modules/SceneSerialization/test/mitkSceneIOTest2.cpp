/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkDataStorageCompare.h>
#include <mitkIOUtil.h>
#include <mitkSceneIO.h>
#include <mitkSceneIOTestScenarioProvider.h>

#include <mitkCoreServices.h>
#include <mitkDataNode.h>
#include <mitkIPropertyMigration.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkSurface.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <itksys/SystemTools.hxx>

#include <limits>

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
  MITK_TEST(Test_SelectedPropertyNotPersisted);
  MITK_TEST(Test_SelectedPropertyNotPersistedForDataLessNode);
  MITK_TEST(Test_LegacyPropertiesMigratedOnLoad);
  MITK_TEST(Test_SurfaceTimeKept);
  MITK_TEST(Test_StaticSurfaceWithTimePropertiesStaysStatic);
  MITK_TEST(Test_RemovedParentKeepsGrandchildBelowGrandparent);
  CPPUNIT_TEST_SUITE_END();

  mitk::SceneIOTestScenarioProvider m_TestCaseProvider;
  std::string m_Directory;

  static mitk::DataNode::Pointer CreateSurfaceNode(const std::string &name)
  {
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(0.0, 0.0, 0.0);
    points->InsertNextPoint(1.0, 0.0, 0.0);
    points->InsertNextPoint(0.0, 1.0, 0.0);

    const vtkIdType ids[] = { 0, 1, 2 };
    auto polys = vtkSmartPointer<vtkCellArray>::New();
    polys->InsertNextCell(3, ids);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polys);

    auto surface = mitk::Surface::New();
    surface->SetVtkPolyData(polyData);

    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetData(surface);
    return node;
  }

public:
  void setUp() override
  {
    m_Directory = mitk::IOUtil::CreateTemporaryDirectory("SceneIOTest_XXXXXX");
  }

  void tearDown() override
  {
    itksys::SystemTools::RemoveADirectory(m_Directory);
  }

  // The scene comparison does not compare time geometries.
  void Test_SurfaceTimeKept()
  {
    std::string archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);

    auto timelessNode = CreateSurfaceNode("timeless");

    // Not representable as floats, like the time points of DICOM series.
    auto timedNode = CreateSurfaceNode("timed");
    auto *timeGeometry = dynamic_cast<mitk::ProportionalTimeGeometry *>(timedNode->GetData()->GetTimeGeometry());
    timeGeometry->SetFirstTimePoint(1234.567);
    timeGeometry->SetStepDuration(1234.567);

    auto originalStorage = mitk::StandaloneDataStorage::New();
    originalStorage->Add(timelessNode);
    originalStorage->Add(timedNode);

    mitk::SceneIO::Pointer writer = mitk::SceneIO::New();
    CPPUNIT_ASSERT(writer->SaveScene(originalStorage->GetAll(), originalStorage, archiveFilename));

    mitk::SceneIO::Pointer reader = mitk::SceneIO::New();
    mitk::DataStorage::Pointer restoredStorage;
    CPPUNIT_ASSERT_NO_THROW(restoredStorage = reader->LoadScene(archiveFilename));

    const auto *restoredTimeless = restoredStorage->GetNamedNode("timeless")->GetData();
    CPPUNIT_ASSERT(restoredTimeless->GetTimeGeometry()->IsValidAtEveryTimePoint());

    const auto timeBounds = restoredStorage->GetNamedNode("timed")->GetData()->GetTimeGeometry()->GetTimeBounds(0);
    CPPUNIT_ASSERT_EQUAL(1234.567, timeBounds[0]);
    CPPUNIT_ASSERT_EQUAL(1234.567 + 1234.567, timeBounds[1]);
  }

  void Test_RemovedParentKeepsGrandchildBelowGrandparent()
  {
    // Like a polygon model of a label whose segmentation was removed.
    auto image = CreateSurfaceNode("image");
    auto segmentation = CreateSurfaceNode("segmentation");
    auto polygonModel = CreateSurfaceNode("polygon model");

    auto originalStorage = mitk::StandaloneDataStorage::New();
    originalStorage->Add(image);
    originalStorage->Add(segmentation, image);
    originalStorage->Add(polygonModel, segmentation);
    originalStorage->Remove(segmentation);

    const auto archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);
    CPPUNIT_ASSERT(mitk::SceneIO::New()->SaveScene(originalStorage->GetAll(), originalStorage, archiveFilename));

    mitk::DataStorage::Pointer restoredStorage;
    CPPUNIT_ASSERT_NO_THROW(restoredStorage = mitk::SceneIO::New()->LoadScene(archiveFilename));

    const auto sources = restoredStorage->GetSources(restoredStorage->GetNamedNode("polygon model"));
    CPPUNIT_ASSERT_EQUAL(1u, sources->Size());
    CPPUNIT_ASSERT(sources->GetElement(0) == restoredStorage->GetNamedNode("image"));
  }
  void Test_StaticSurfaceWithTimePropertiesStaysStatic()
  {
    // Older versions of MITK stamped the time of some surfaces, like those of
    // the 3D interpolation, as float properties, in which the first time point
    // of a surface that is valid at every time point is -inf.
    auto node = CreateSurfaceNode("static");
    node->GetData()->SetProperty("ProportionalTimeGeometry.FirstTimePoint",
                                 mitk::FloatProperty::New(-std::numeric_limits<float>::infinity()));
    node->GetData()->SetProperty("ProportionalTimeGeometry.StepDuration",
                                 mitk::FloatProperty::New(std::numeric_limits<float>::infinity()));

    auto originalStorage = mitk::StandaloneDataStorage::New();
    originalStorage->Add(node);

    const auto archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);
    CPPUNIT_ASSERT(mitk::SceneIO::New()->SaveScene(originalStorage->GetAll(), originalStorage, archiveFilename));

    mitk::DataStorage::Pointer restoredStorage;
    CPPUNIT_ASSERT_NO_THROW(restoredStorage = mitk::SceneIO::New()->LoadScene(archiveFilename));
    CPPUNIT_ASSERT(restoredStorage->GetNamedNode("static")->GetData()->GetTimeGeometry()->IsValidAtEveryTimePoint());
  }

  void Test_SceneIOInterfaces() { CPPUNIT_ASSERT_MESSAGE("Not urgent", true); }
  void Test_ReconstructionOfScenes()
  {

    mitk::SceneIOTestScenarioProvider::ScenarioList scenarios = m_TestCaseProvider.GetAllScenarios();
    MITK_TEST_OUTPUT(<< "Executing " << scenarios.size() << " test scenarios");
    for (const auto& scenario : scenarios)
    {
      MITK_TEST_OUTPUT(<< "\n===== Test_ReconstructionOfScenes, scenario '" << scenario.key << "' =====");

      std::string archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);
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

  void Test_SelectedPropertyNotPersisted()
  {
    std::string archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);

    auto originalStorage = mitk::StandaloneDataStorage::New();
    auto node = mitk::DataNode::New();
    node->SetData(mitk::PointSet::New());
    node->SetName("measurement");
    node->SetSelected(true); // transient UI state
    originalStorage->Add(node);

    mitk::SceneIO::Pointer writer = mitk::SceneIO::New();
    CPPUNIT_ASSERT_MESSAGE("Saving scene with a selected node",
                           writer->SaveScene(originalStorage->GetAll(), originalStorage, archiveFilename));

    mitk::SceneIO::Pointer reader = mitk::SceneIO::New();
    mitk::DataStorage::Pointer restoredStorage;
    CPPUNIT_ASSERT_NO_THROW(restoredStorage = reader->LoadScene(archiveFilename));

    mitk::DataNode::Pointer restoredNode = restoredStorage->GetNamedNode("measurement");
    CPPUNIT_ASSERT_MESSAGE("Reloaded node must exist", restoredNode.IsNotNull());

    bool selected = false;
    bool hasSelected = restoredNode->GetBoolProperty("selected", selected);
    CPPUNIT_ASSERT_MESSAGE("Transient 'selected' must not be persisted", !hasSelected || !selected);
    CPPUNIT_ASSERT_MESSAGE("Non-transient 'name' must survive", restoredNode->GetName() == "measurement");
  }

  void Test_SelectedPropertyNotPersistedForDataLessNode()
  {
    // "selected" is registered transient for any BaseData type, including
    // data-less nodes (GetData() == nullptr). Filtering must not be skipped
    // just because the node carries no data.
    std::string archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);

    auto originalStorage = mitk::StandaloneDataStorage::New();
    auto node = mitk::DataNode::New(); // no SetData: a data-less node
    node->SetName("group");
    node->SetSelected(true); // transient UI state
    originalStorage->Add(node);

    mitk::SceneIO::Pointer writer = mitk::SceneIO::New();
    CPPUNIT_ASSERT_MESSAGE("Saving scene with a selected data-less node",
                           writer->SaveScene(originalStorage->GetAll(), originalStorage, archiveFilename));

    mitk::SceneIO::Pointer reader = mitk::SceneIO::New();
    mitk::DataStorage::Pointer restoredStorage;
    CPPUNIT_ASSERT_NO_THROW(restoredStorage = reader->LoadScene(archiveFilename));

    mitk::DataNode::Pointer restoredNode = restoredStorage->GetNamedNode("group");
    CPPUNIT_ASSERT_MESSAGE("Reloaded data-less node must exist", restoredNode.IsNotNull());

    bool selected = false;
    bool hasSelected = restoredNode->GetBoolProperty("selected", selected);
    CPPUNIT_ASSERT_MESSAGE("Transient 'selected' must not be persisted for a data-less node",
                           !hasSelected || !selected);
    CPPUNIT_ASSERT_MESSAGE("Non-transient 'name' must survive", restoredNode->GetName() == "group");
  }

  void Test_LegacyPropertiesMigratedOnLoad()
  {
    // The keys are unique to this test, since migrations stay registered for
    // the lifetime of the process.
    mitk::CoreServicePointer<mitk::IPropertyMigration> migration(mitk::CoreServices::GetPropertyMigration());
    migration->AddMigration<mitk::PointSet>("sceneiotest.legacy width",
      [](const mitk::BaseData *, const mitk::BaseProperty &legacyProperty, mitk::PropertyList &properties) {
        properties.SetProperty("sceneiotest.width", legacyProperty.Clone());
      });
    migration->AddRemoval<mitk::PointSet>("sceneiotest.legacy flag");

    std::string archiveFilename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);

    auto originalStorage = mitk::StandaloneDataStorage::New();
    auto node = mitk::DataNode::New();
    auto pointSet = mitk::PointSet::New();
    mitk::Point3D point;
    point.Fill(1.0);
    pointSet->InsertPoint(0, point); // empty point sets are not saved
    pointSet->GetPropertyList()->SetBoolProperty("sceneiotest.legacy flag", true);
    node->SetData(pointSet);
    node->SetName("legacy");
    node->SetFloatProperty("sceneiotest.legacy width", 4.0f);
    node->GetPropertyList("stdmulti.widget0")->SetFloatProperty("sceneiotest.legacy width", 5.0f);
    originalStorage->Add(node);

    mitk::SceneIO::Pointer writer = mitk::SceneIO::New();
    CPPUNIT_ASSERT_MESSAGE("Saving scene with legacy properties",
                           writer->SaveScene(originalStorage->GetAll(), originalStorage, archiveFilename));

    mitk::SceneIO::Pointer reader = mitk::SceneIO::New();
    mitk::DataStorage::Pointer restoredStorage;
    CPPUNIT_ASSERT_NO_THROW(restoredStorage = reader->LoadScene(archiveFilename));

    mitk::DataNode::Pointer restoredNode = restoredStorage->GetNamedNode("legacy");
    CPPUNIT_ASSERT_MESSAGE("Reloaded node must exist", restoredNode.IsNotNull());

    float width = 0.0f;
    CPPUNIT_ASSERT_MESSAGE("Node property is migrated", restoredNode->GetFloatProperty("sceneiotest.width", width));
    CPPUNIT_ASSERT_EQUAL(4.0f, width);
    CPPUNIT_ASSERT_MESSAGE("Legacy node property is gone", restoredNode->GetProperty("sceneiotest.legacy width") == nullptr);

    auto *rendererList = restoredNode->GetPropertyList("stdmulti.widget0");
    CPPUNIT_ASSERT_MESSAGE("Render-window property is migrated", rendererList->GetFloatProperty("sceneiotest.width", width));
    CPPUNIT_ASSERT_EQUAL(5.0f, width);
    CPPUNIT_ASSERT_MESSAGE("Legacy render-window property is gone", rendererList->GetProperty("sceneiotest.legacy width") == nullptr);

    CPPUNIT_ASSERT_MESSAGE("Legacy data property is gone",
                           restoredNode->GetData()->GetProperty("sceneiotest.legacy flag").IsNull());
  }

}; // class

int mitkSceneIOTest2(int /*argc*/, char * /*argv*/ [])
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(mitkSceneIOTest2Suite::suite());
  return runner.run() ? 0 : 1;
}
