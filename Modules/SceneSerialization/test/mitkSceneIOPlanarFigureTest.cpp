/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkColorProperty.h>
#include <mitkCoreServices.h>
#include <mitkDataNode.h>
#include <mitkIOUtil.h>
#include <mitkIPropertyMigration.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureVisibilityProperty.h>
#include <mitkPlanarLine.h>
#include <mitkPropertyList.h>
#include <mitkSceneIO.h>

#include <itksys/SystemTools.hxx>

#include <set>

namespace
{
  // Properties of previous versions of planar figures that no current list keeps.
  bool IsLegacyKey(const std::string &key)
  {
    static const std::set<std::string> currentKeys = {
      "planarfigure.drawname", "planarfigure.drawquantities", "planarfigure.label", "planarfigure.helperlines",
      "planarfigure.drawcontrastoutline", "planarfigure.3drendering", "planarfigure.3drendering.fill"
    };

    static const std::set<std::string> legacyDataKeys = {
      "ClosedPlanarPolygon", "closed", "initiallyplaced", "subdivision", "SingleLineMode"
    };

    return (0 == key.rfind("planarfigure.", 0) && 0 == currentKeys.count(key)) || 0 != legacyDataKeys.count(key);
  }

  std::string GetLegacyKeys(const mitk::DataNode *node)
  {
    std::string legacyKeys;

    const auto collect = [&legacyKeys](const std::vector<std::string> &keys) {
      for (const auto &key : keys)
      {
        if (IsLegacyKey(key))
          legacyKeys += " " + key;
      }
    };

    collect(node->GetPropertyKeys());

    for (const auto &context : node->GetPropertyContextNames())
      collect(node->GetPropertyKeys(context));

    collect(node->GetData()->GetPropertyKeys());

    return legacyKeys;
  }
}

/**
 * \brief Loads a scene with every type of planar figure that a previous
 * version of MITK wrote, and saves and loads it again.
 */
class mitkSceneIOPlanarFigureTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSceneIOPlanarFigureTestSuite);
  MITK_TEST(LoadLegacyScene_ConvertsFigures);
  MITK_TEST(LoadLegacyScene_RemovesLegacyProperties);
  MITK_TEST(LoadLegacyScene_MigratesStyle);
  MITK_TEST(LoadLegacyScene_AddsCurrentDefaults);
  MITK_TEST(MigrateDefaultLineColor_BecomesColor);
  MITK_TEST(LoadLegacyScene_KeepsSegmentationProperties);
  MITK_TEST(SaveAndLoad_KeepsFigures);
  MITK_TEST(SaveAndLoad_KeepsValuesOfPreviousDefaults);
  CPPUNIT_TEST_SUITE_END();

  std::string m_Directory;
  mitk::DataStorage::Pointer m_Storage;

  mitk::DataNode *GetFigureNode(const std::string &name) const
  {
    auto *node = m_Storage->GetNamedNode(name);

    CPPUNIT_ASSERT_MESSAGE(name, nullptr != node && nullptr != dynamic_cast<mitk::PlanarFigure *>(node->GetData()));
    return node;
  }

  std::vector<mitk::DataNode *> GetFigureNodes(const mitk::DataStorage *storage) const
  {
    std::vector<mitk::DataNode *> nodes;
    const auto allNodes = storage->GetAll();

    for (const auto &node : *allNodes)
    {
      if (nullptr != dynamic_cast<mitk::PlanarFigure *>(node->GetData()))
        nodes.push_back(node);
    }

    return nodes;
  }

public:
  void setUp() override
  {
    m_Directory = mitk::IOUtil::CreateTemporaryDirectory("SceneIOPlanarFigureTest_XXXXXX");
    m_Storage = mitk::SceneIO::New()->LoadScene(GetTestDataFilePath("SceneSerialization/PlanarFigures/LegacyPlanarFigures.mitk"));
  }

  void tearDown() override
  {
    m_Storage = nullptr;
    itksys::SystemTools::RemoveADirectory(m_Directory);
  }

  void LoadLegacyScene_ConvertsFigures()
  {
    // An open polygon of a previous version is a path now.
    const std::vector<std::pair<std::string, std::string>> classes = {
      { "Line1", "PlanarLine" },
      { "Angle1", "PlanarAngle" },
      { "Four Point Angle1", "PlanarFourPointAngle" },
      { "Cross1", "PlanarCross" },
      { "Circle1", "PlanarCircle" },
      { "Circle2", "PlanarCircle" },
      { "Ellipse1", "PlanarEllipse" },
      { "DoubleEllipse1", "PlanarDoubleEllipse" },
      { "DoubleEllipse2", "PlanarDoubleEllipse" },
      { "Rectangle1", "PlanarRectangle" },
      { "Polygon1", "PlanarPolygon" },
      { "Path1", "PlanarPath" },
      { "SubdivisionPolygon1", "PlanarSubdivisionPolygon" },
      { "BezierCurve1", "PlanarBezierCurve" }
    };

    CPPUNIT_ASSERT_EQUAL(std::size_t(16), this->GetFigureNodes(m_Storage).size());

    for (const auto &[name, className] : classes)
      CPPUNIT_ASSERT_EQUAL_MESSAGE(name, className, std::string(this->GetFigureNode(name)->GetData()->GetNameOfClass()));
  }

  void LoadLegacyScene_RemovesLegacyProperties()
  {
    for (const auto *node : this->GetFigureNodes(m_Storage))
      CPPUNIT_ASSERT_EQUAL_MESSAGE(node->GetName(), std::string(), GetLegacyKeys(node));
  }

  void LoadLegacyScene_MigratesStyle()
  {
    float lineWidth = 0.0f;
    CPPUNIT_ASSERT(this->GetFigureNode("Circle1")->GetFloatProperty("line width", lineWidth));
    CPPUNIT_ASSERT_EQUAL(4.0f, lineWidth);

    // The defaults of previous versions give way to the current ones.
    const auto *line = this->GetFigureNode("Line1");

    lineWidth = 0.0f;
    CPPUNIT_ASSERT(line->GetFloatProperty("line width", lineWidth));
    CPPUNIT_ASSERT_EQUAL(2.0f, lineWidth);

    float opacity = 1.0f;
    line->GetOpacity(opacity, nullptr);
    CPPUNIT_ASSERT_EQUAL(1.0f, opacity);

    // Name labels were on by default, as they are now, so a switched off one stays off.
    bool drawName = false;
    CPPUNIT_ASSERT(line->GetBoolProperty("planarfigure.drawname", drawName) && drawName);
    CPPUNIT_ASSERT(this->GetFigureNode("Rectangle1")->GetBoolProperty("planarfigure.drawname", drawName) && !drawName);
  }

  void LoadLegacyScene_AddsCurrentDefaults()
  {
    // Properties added since previous versions exist, so they can be changed.
    using Visibility = mitk::PlanarFigureVisibilityProperty::Visibility;

    const auto getVisibility = [this](const std::string &name, const std::string &key) {
      const auto *property = dynamic_cast<const mitk::PlanarFigureVisibilityProperty *>(this->GetFigureNode(name)->GetProperty(key.c_str()));
      CPPUNIT_ASSERT_MESSAGE(name + ": " + key, nullptr != property);
      return property->GetVisibility();
    };

    CPPUNIT_ASSERT(Visibility::OnHover == getVisibility("BezierCurve1", "planarfigure.helperlines"));
    CPPUNIT_ASSERT(Visibility::Always == getVisibility("Line1", "planarfigure.helperlines"));
    CPPUNIT_ASSERT(Visibility::OnHover == getVisibility("Line1", "planarfigure.label"));

    int fontSize = 0;
    CPPUNIT_ASSERT(this->GetFigureNode("Line1")->GetIntProperty("font.size", fontSize));
    CPPUNIT_ASSERT_EQUAL(13, fontSize);
  }

  void MigrateDefaultLineColor_BecomesColor()
  {
    // Previous versions drew a figure in this color instead of its color, which they kept white.
    auto properties = mitk::PropertyList::New();
    properties->SetProperty("color", mitk::ColorProperty::New(1.0f, 1.0f, 1.0f));
    properties->SetProperty("planarfigure.default.line.color", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f));

    mitk::CoreServicePointer<mitk::IPropertyMigration> migration(mitk::CoreServices::GetPropertyMigration());
    migration->Migrate(mitk::PlanarLine::New(), *properties);

    CPPUNIT_ASSERT(nullptr == properties->GetProperty("planarfigure.default.line.color"));

    const auto *color = dynamic_cast<const mitk::ColorProperty *>(properties->GetProperty("color"));
    CPPUNIT_ASSERT(nullptr != color);
    CPPUNIT_ASSERT_EQUAL(1.0f, color->GetColor()[0]);
    CPPUNIT_ASSERT_EQUAL(0.0f, color->GetColor()[1]);
    CPPUNIT_ASSERT_EQUAL(0.0f, color->GetColor()[2]);
  }

  void LoadLegacyScene_KeepsSegmentationProperties()
  {
    const auto *marker = this->GetFigureNode("Contourmarker 1");

    bool isContourMarker = false;
    CPPUNIT_ASSERT(marker->GetBoolProperty("isContourMarker", isContourMarker) && isContourMarker);

    bool isInitializedWindow = false;
    CPPUNIT_ASSERT(marker->GetPropertyList("stdmulti.widget0")->GetBoolProperty("PlanarFigureInitializedWindow", isInitializedWindow));
    CPPUNIT_ASSERT(isInitializedWindow);

    bool isContourPlane = false;
    CPPUNIT_ASSERT(this->GetFigureNode("contourPlane L 1 T 0")->GetBoolProperty("isContourPlaneGeometry", isContourPlane));
    CPPUNIT_ASSERT(isContourPlane);
  }

  void SaveAndLoad_KeepsFigures()
  {
    const auto filename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);
    CPPUNIT_ASSERT(mitk::SceneIO::New()->SaveScene(m_Storage->GetAll(), m_Storage, filename));

    const auto restoredStorage = mitk::SceneIO::New()->LoadScene(filename);

    for (const auto *node : this->GetFigureNodes(m_Storage))
    {
      const auto *restoredNode = restoredStorage->GetNamedNode(node->GetName());
      CPPUNIT_ASSERT_MESSAGE(node->GetName(), nullptr != restoredNode);

      const auto *figure = static_cast<const mitk::PlanarFigure *>(node->GetData());
      const auto *restoredFigure = dynamic_cast<const mitk::PlanarFigure *>(restoredNode->GetData());

      CPPUNIT_ASSERT_MESSAGE(node->GetName(), nullptr != restoredFigure && mitk::Equal(*figure, *restoredFigure, mitk::eps, true));
      CPPUNIT_ASSERT_EQUAL_MESSAGE(node->GetName(), std::string(), GetLegacyKeys(restoredNode));
    }

    float lineWidth = 0.0f;
    CPPUNIT_ASSERT(restoredStorage->GetNamedNode("Circle1")->GetFloatProperty("line width", lineWidth));
    CPPUNIT_ASSERT_EQUAL(4.0f, lineWidth);
  }

  void SaveAndLoad_KeepsValuesOfPreviousDefaults()
  {
    // In a current scene, this is a choice rather than a default of a previous version.
    auto *line = this->GetFigureNode("Line1");
    line->SetOpacity(0.8f);

    const auto filename = mitk::IOUtil::CreateTemporaryFile("scene_XXXXXX.mitk", m_Directory);
    CPPUNIT_ASSERT(mitk::SceneIO::New()->SaveScene(m_Storage->GetAll(), m_Storage, filename));

    const auto restoredStorage = mitk::SceneIO::New()->LoadScene(filename);
    const auto *restoredLine = restoredStorage->GetNamedNode("Line1");

    float opacity = 1.0f;
    CPPUNIT_ASSERT(restoredLine->GetOpacity(opacity, nullptr));
    CPPUNIT_ASSERT_EQUAL(0.8f, opacity);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSceneIOPlanarFigure)
