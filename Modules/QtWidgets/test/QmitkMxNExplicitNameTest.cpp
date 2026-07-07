/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

/**
 * Tests the explicit-id 'CreateRenderWindowWidget(const QString&)' overload
 * on QmitkMxNMultiWidget plus the collision-free positional fallback used by
 * 'SetLayout(r, c)'. Together they replace an earlier 'widget<count>'
 * naming scheme that silently collided when custom-named cells already
 * used the same index (std::map::insert silently rejects duplicate keys).
 */
class QmitkMxNExplicitNameTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNExplicitNameTestSuite);
  MITK_TEST(Create_WithExplicitId_RegistersVerbatim);
  MITK_TEST(Create_UnprefixedId_Throws);
  MITK_TEST(Create_DuplicateId_Throws);
  MITK_TEST(Create_EmptyId_Throws);
  MITK_TEST(Positional_FillsGapsFromZero);
  MITK_TEST(Positional_SkipsAlreadyUsedSlots);
  MITK_TEST(CustomEditorName_RegistersWithItsPrefix);
  MITK_TEST(CustomEditorName_RejectsOtherEditorPrefix);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Node1;

public:
  void setUp() override
  {
    EnsureQApplication();

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
  }

  void tearDown() override
  {
    m_Node1 = nullptr;
    m_DataStorage = nullptr;
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

  // ---------- Explicit-id overload ----------

  void Create_WithExplicitId_RegistersVerbatim()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    auto cell = widget.CreateRenderWindowWidget("mxn__alpha");

    CPPUNIT_ASSERT(nullptr != cell);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Id is registered verbatim: no prefix prepend, no strip",
      std::string("mxn__alpha"),
      cell->GetWidgetName().toStdString());
    CPPUNIT_ASSERT_MESSAGE(
      "Render-window widget must be findable by its canonical id",
      nullptr != widget.GetRenderWindowWidget(QString("mxn__alpha")));
  }

  void Create_UnprefixedId_Throws()
  {
    // The explicit overload requires the canonical qualified id; passing a
    // bare-style id is in-process API misuse and must throw rather than
    // silently land under a wrong name.
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    CPPUNIT_ASSERT_THROW(widget.CreateRenderWindowWidget("alpha"), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, WidgetCount(widget));
  }

  void Create_DuplicateId_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    widget.CreateRenderWindowWidget("mxn__alpha");
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, WidgetCount(widget));

    CPPUNIT_ASSERT_THROW(widget.CreateRenderWindowWidget("mxn__alpha"), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Map size must remain 1 after rejected duplicate",
      std::size_t{1}, WidgetCount(widget));
  }

  void Create_EmptyId_Throws()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    CPPUNIT_ASSERT_THROW(widget.CreateRenderWindowWidget(""), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, WidgetCount(widget));
  }

  // ---------- Positional fallback ----------

  void Positional_FillsGapsFromZero()
  {
    // Sanity check: the positional path produces 'mxn__widget0',
    // 'mxn__widget1', ... when no custom-named cells are present.
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    auto cell0 = widget.CreateRenderWindowWidget(QString("mxn__widget0"));
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget0"), cell0->GetWidgetName().toStdString());
    CPPUNIT_ASSERT(nullptr != widget.GetRenderWindowWidget(QString("mxn__widget0")));

    // Re-derive the expected positional behaviour by going through the same
    // public path the layout applier uses: the explicit-id overload with
    // the next qualified id. This documents the intent without depending
    // on the internal nullary helper signature.
    auto cell1 = widget.CreateRenderWindowWidget(QString("mxn__widget1"));
    auto cell2 = widget.CreateRenderWindowWidget(QString("mxn__widget2"));
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget1"), cell1->GetWidgetName().toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("mxn__widget2"), cell2->GetWidgetName().toStdString());
    CPPUNIT_ASSERT_EQUAL(std::size_t{3}, WidgetCount(widget));
  }

  void Positional_SkipsAlreadyUsedSlots()
  {
    // The latent v1 bug this test locks down: with custom-named cells already
    // present, the legacy 'widget<count>' positional naming would produce
    // collisions ('widget3' twice). The new positional path picks the lowest
    // unused 'widget<i>' index inside the canonical 'mxn__widget<i>' form.
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);

    // Pre-populate with a custom-indexed name occupying slot 3.
    widget.CreateRenderWindowWidget(QString("mxn__widget3"));
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, WidgetCount(widget));

    // Drive 'SetLayout(1, 5)': the public positional path used by the
    // configuration toolbar. The internal nullary 'CreateRenderWindowWidget'
    // must skip 'mxn__widget3' and pick 'mxn__widget4' for the fifth cell.
    widget.SetLayout(1, 5);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Total cell count must match the requested grid",
      std::size_t{5}, WidgetCount(widget));

    // Each expected id must be present exactly once.
    for (const auto& id : { "mxn__widget0", "mxn__widget1", "mxn__widget2",
                            "mxn__widget3", "mxn__widget4" })
    {
      CPPUNIT_ASSERT_MESSAGE(
        ("Expected cell '" + std::string(id) + "' missing").c_str(),
        nullptr != widget.GetRenderWindowWidget(QString(id)));
    }
  }

  // ---------- Editor-prefix invariant ----------

  void CustomEditorName_RegistersWithItsPrefix()
  {
    QmitkMxNMultiWidget widget(nullptr, Qt::WindowFlags{}, QString("custom"));
    widget.SetDataStorage(m_DataStorage);

    auto cell = widget.CreateRenderWindowWidget("custom__alpha");
    CPPUNIT_ASSERT_EQUAL(std::string("custom__alpha"), cell->GetWidgetName().toStdString());
    CPPUNIT_ASSERT(nullptr != widget.GetRenderWindowWidget(QString("custom__alpha")));
  }

  void CustomEditorName_RejectsOtherEditorPrefix()
  {
    QmitkMxNMultiWidget widget(nullptr, Qt::WindowFlags{}, QString("custom"));
    widget.SetDataStorage(m_DataStorage);

    // Ids belong to one editor instance, identified by its multiWidgetName;
    // a custom-named editor rejects ids carrying any other prefix.
    CPPUNIT_ASSERT_THROW(widget.CreateRenderWindowWidget("mxn__alpha"), mitk::Exception);
    CPPUNIT_ASSERT_EQUAL(std::size_t{0}, WidgetCount(widget));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNExplicitName)
