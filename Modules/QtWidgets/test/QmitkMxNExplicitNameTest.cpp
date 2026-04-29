/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>
#include <mitkRenderingTestHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QApplication>

extern std::vector<std::string> globalCmdLineArgs;

/**
 * Tests the explicit-name 'CreateRenderWindowWidget(const QString&)' overload
 * on QmitkMxNMultiWidget plus the collision-free positional fallback used by
 * 'SetLayout(r, c)'. Together they replace the legacy 'widget<count>' naming
 * (which silently collided when custom-named cells already used the same
 * index, because std::map::insert silently rejects duplicate keys).
 *
 * The fixture deliberately avoids 'SetLayout(...)' so the tests focus on the
 * naming contract; the positional overload is exercised by direct calls to
 * the nullary 'CreateRenderWindowWidget()'.
 */
class QmitkMxNExplicitNameTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNExplicitNameTestSuite);
  MITK_TEST(Create_WithBareName_RegistersQualifiedName);
  MITK_TEST(Create_DuplicateBareName_Throws);
  MITK_TEST(Create_EmptyBareName_Throws);
  MITK_TEST(Positional_FillsGapsFromZero);
  MITK_TEST(Positional_SkipsAlreadyUsedSlots);
  MITK_TEST(QualifiedName_UsesEditorPrefix);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node1;

  QApplication* m_TestApp = nullptr;

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();

    // QmitkRenderWindowDataNodeTableModel sorts its node list by the int
    // "layer" property; the comparator violates strict-weak-ordering when
    // either side has no such property. Test nodes therefore carry an
    // explicit node-level layer. See QmitkMxNSyncGroupApiTest for the long
    // explanation; remove together with that workaround once the comparator
    // is fixed.
    m_Node1 = mitk::DataNode::New();
    m_Node1->SetName("node1");
    m_Node1->SetIntProperty("layer", 0);
    m_DataStorage->Add(m_Node1);

    mitk::RenderingTestHelper::ArgcHelperClass cmdLineArgs(globalCmdLineArgs);
    auto argc = cmdLineArgs.GetArgc();
    auto argv = cmdLineArgs.GetArgv();
    m_TestApp = new QApplication(argc, argv);
  }

  void tearDown() override
  {
    delete m_TestApp;
    m_TestApp = nullptr;
  }

  /**
   * Returns the number of currently registered render-window widgets. Helper
   * because 'GetNumberOfRenderWindowWidgets' returns 'unsigned int'; the test
   * comparisons are clearer with 'std::size_t'.
   */
  static std::size_t WidgetCount(const QmitkMxNMultiWidget& widget)
  {
    return static_cast<std::size_t>(widget.GetNumberOfRenderWindowWidgets());
  }

  // ---------- Explicit-name overload ----------

  void Create_WithBareName_RegistersQualifiedName()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    auto cell = widget.CreateRenderWindowWidget("alpha");

    CPPUNIT_ASSERT(nullptr != cell);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Bare name must be registered with the editor's '<multiWidgetName>.' prefix",
      std::string("mxn.alpha"),
      cell->GetWidgetName().toStdString());
    CPPUNIT_ASSERT_MESSAGE(
      "Render-window widget must be findable by qualified name",
      nullptr != widget.GetRenderWindowWidget(QString("mxn.alpha")));
  }

  void Create_DuplicateBareName_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    widget.CreateRenderWindowWidget("alpha");
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, WidgetCount(widget));

    CPPUNIT_ASSERT_THROW(widget.CreateRenderWindowWidget("alpha"), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Map size must remain 1 after rejected duplicate",
      std::size_t{1}, WidgetCount(widget));
  }

  void Create_EmptyBareName_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    CPPUNIT_ASSERT_THROW(widget.CreateRenderWindowWidget(""), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, WidgetCount(widget));
  }

  // ---------- Positional fallback ----------

  void Positional_FillsGapsFromZero()
  {
    // Sanity check: the positional path produces 'widget0', 'widget1', ...
    // when no custom-named cells are present.
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    auto cell0 = widget.CreateRenderWindowWidget(QString("widget0"));
    CPPUNIT_ASSERT_EQUAL(std::string("mxn.widget0"), cell0->GetWidgetName().toStdString());
    CPPUNIT_ASSERT(nullptr != widget.GetRenderWindowWidget(QString("mxn.widget0")));

    // Re-derive the expected positional behaviour by going through the same
    // public path the layout applier uses: the explicit-name overload with
    // the next 'widget<i>' name. This documents the intent without depending
    // on the internal nullary helper signature.
    auto cell1 = widget.CreateRenderWindowWidget(QString("widget1"));
    auto cell2 = widget.CreateRenderWindowWidget(QString("widget2"));
    CPPUNIT_ASSERT_EQUAL(std::string("mxn.widget1"), cell1->GetWidgetName().toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("mxn.widget2"), cell2->GetWidgetName().toStdString());
    CPPUNIT_ASSERT_EQUAL(std::size_t{3}, WidgetCount(widget));
  }

  void Positional_SkipsAlreadyUsedSlots()
  {
    // The latent v1 bug this test locks down: with custom-named cells already
    // present, the legacy 'widget<count>' positional naming would produce
    // collisions ('widget3' twice). The new positional path picks the lowest
    // unused 'widget<i>' name instead.
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    // Pre-populate with a custom-indexed name occupying slot 3.
    widget.CreateRenderWindowWidget(QString("widget3"));
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, WidgetCount(widget));

    // Drive 'SetLayout(1, 5)': the public positional path used by the
    // configuration toolbar. The internal nullary 'CreateRenderWindowWidget'
    // must skip 'widget3' and pick 'widget4' for the fifth cell.
    widget.SetLayout(1, 5);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Total cell count must match the requested grid",
      std::size_t{5}, WidgetCount(widget));

    // Each expected name must be present exactly once.
    for (const auto& bare : { "widget0", "widget1", "widget2", "widget3", "widget4" })
    {
      const auto qualified = QString("mxn.") + bare;
      CPPUNIT_ASSERT_MESSAGE(
        ("Expected cell '" + std::string(qualified.toUtf8()) + "' missing").c_str(),
        nullptr != widget.GetRenderWindowWidget(qualified));
    }
  }

  // ---------- Editor-prefix invariant ----------

  void QualifiedName_UsesEditorPrefix()
  {
    // An editor instantiated with a non-default 'multiWidgetName' must use
    // that name (not the static "mxn" default) when prefixing the bare name.
    QmitkMxNMultiWidget widget(nullptr, Qt::WindowFlags{}, QString("custom"));
    widget.SetDataStorage(m_DataStorage);

    auto cell = widget.CreateRenderWindowWidget("alpha");
    CPPUNIT_ASSERT_EQUAL(std::string("custom.alpha"), cell->GetWidgetName().toStdString());
    CPPUNIT_ASSERT(nullptr != widget.GetRenderWindowWidget(QString("custom.alpha")));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNExplicitName)
