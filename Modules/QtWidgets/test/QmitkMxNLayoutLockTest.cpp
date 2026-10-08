/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNMultiWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QDir>

#include <fstream>
#include <vector>

/**
 * Tests the MxN layout lock: the declaration parsers that turn plugin.xml
 * values into a lock set and a widget name, and the lock state on the widget.
 *
 * Also applies the layouts the example plugin org.mitk.example.gui.mxneditor
 * ships, whose resource directory is the last command-line argument. Nothing
 * starts that example automatically, so this is what catches window-ID or
 * schema drift in its files.
 */
class QmitkMxNLayoutLockTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLayoutLockTestSuite);
  MITK_TEST(Parse_EachTokenAlone);
  MITK_TEST(Parse_AllTokens);
  MITK_TEST(Parse_TrimsWhitespace);
  MITK_TEST(Parse_EmptyOrBlank_YieldsNoLock);
  MITK_TEST(Parse_UnknownToken_Throws);
  MITK_TEST(Parse_WrongCase_Throws);
  MITK_TEST(Parse_EmptyToken_Throws);
  MITK_TEST(ValidName_Accepted);
  MITK_TEST(InvalidName_Rejected);
  MITK_TEST(Locks_DefaultToNone);
  MITK_TEST(Signal_FiresOncePerActualChange);
  MITK_TEST(ExampleLayouts_ApplyToTheDemoEditor);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;

public:
  void setUp() override
  {
    EnsureQApplication();
    m_DataStorage = mitk::StandaloneDataStorage::New();
  }

  void tearDown() override
  {
    m_DataStorage = nullptr;
  }

  void Parse_EachTokenAlone()
  {
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks(QmitkMxNLayoutLock::Structure) == QmitkMxNParseLayoutLocks("structure"));
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks(QmitkMxNLayoutLock::Direction) == QmitkMxNParseLayoutLocks("direction"));
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks(QmitkMxNLayoutLock::Proportions) == QmitkMxNParseLayoutLocks("proportions"));
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks(QmitkMxNLayoutLock::Content) == QmitkMxNParseLayoutLocks("content"));
  }

  void Parse_AllTokens()
  {
    const auto expected = QmitkMxNLayoutLock::Structure | QmitkMxNLayoutLock::Direction
                        | QmitkMxNLayoutLock::Proportions | QmitkMxNLayoutLock::Content;
    CPPUNIT_ASSERT(expected == QmitkMxNParseLayoutLocks("structure,direction,proportions,content"));
  }

  void Parse_TrimsWhitespace()
  {
    const auto expected = QmitkMxNLayoutLock::Structure | QmitkMxNLayoutLock::Content;
    CPPUNIT_ASSERT(expected == QmitkMxNParseLayoutLocks("  structure ,\tcontent  "));
  }

  void Parse_EmptyOrBlank_YieldsNoLock()
  {
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks() == QmitkMxNParseLayoutLocks(""));
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks() == QmitkMxNParseLayoutLocks("   "));
  }

  void Parse_UnknownToken_Throws()
  {
    CPPUNIT_ASSERT_THROW(QmitkMxNParseLayoutLocks("structure,maximize"), mitk::Exception);
  }

  void Parse_WrongCase_Throws()
  {
    CPPUNIT_ASSERT_THROW(QmitkMxNParseLayoutLocks("Structure"), mitk::Exception);
  }

  void Parse_EmptyToken_Throws()
  {
    CPPUNIT_ASSERT_THROW(QmitkMxNParseLayoutLocks("structure,,direction"), mitk::Exception);
    CPPUNIT_ASSERT_THROW(QmitkMxNParseLayoutLocks("structure,"), mitk::Exception);
  }

  void ValidName_Accepted()
  {
    CPPUNIT_ASSERT(QmitkMxNIsValidMultiWidgetName("mxn"));
    CPPUNIT_ASSERT(QmitkMxNIsValidMultiWidgetName("org.mitk.editors.longitrack"));
    CPPUNIT_ASSERT(QmitkMxNIsValidMultiWidgetName("a-b.c"));
  }

  void InvalidName_Rejected()
  {
    CPPUNIT_ASSERT(!QmitkMxNIsValidMultiWidgetName(""));
    CPPUNIT_ASSERT(!QmitkMxNIsValidMultiWidgetName("1abc"));
    CPPUNIT_ASSERT(!QmitkMxNIsValidMultiWidgetName("a_b"));
    CPPUNIT_ASSERT(!QmitkMxNIsValidMultiWidgetName("a b"));
  }

  void Locks_DefaultToNone()
  {
    QmitkMxNMultiWidget widget;
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks() == widget.GetLayoutLocks());
  }

  void Signal_FiresOncePerActualChange()
  {
    QmitkMxNMultiWidget widget;
    widget.SetDataStorage(m_DataStorage);
    widget.InitializeMultiWidget();
    std::vector<QmitkMxNLayoutLocks> emitted;
    QObject::connect(&widget, &QmitkMxNMultiWidget::LayoutLocksChanged,
                     [&emitted](QmitkMxNLayoutLocks locks) { emitted.push_back(locks); });

    const QmitkMxNLayoutLocks locks = QmitkMxNLayoutLock::Structure | QmitkMxNLayoutLock::Direction;
    widget.SetLayoutLocks(locks);
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, emitted.size());
    CPPUNIT_ASSERT(locks == emitted[0]);
    CPPUNIT_ASSERT(locks == widget.GetLayoutLocks());

    widget.SetLayoutLocks(locks);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Setting the current locks again must not emit", std::size_t{1}, emitted.size());

    widget.SetLayoutLocks(QmitkMxNLayoutLocks());
    CPPUNIT_ASSERT_EQUAL(std::size_t{2}, emitted.size());
    CPPUNIT_ASSERT(QmitkMxNLayoutLocks() == emitted[1]);
  }

  void ExampleLayouts_ApplyToTheDemoEditor()
  {
    CPPUNIT_ASSERT_MESSAGE("The example's resource directory must be passed as the last argument",
                           !globalCmdLineArgs.empty());
    const QDir resources(QString::fromStdString(globalCmdLineArgs.back()));

    // Named and locked as the example's declaration does.
    QmitkMxNMultiWidget widget(nullptr, {}, QStringLiteral("org.mitk.example.editors.mxndemo"));
    widget.SetDataStorage(m_DataStorage);
    widget.SetLayoutLocks(QmitkMxNParseLayoutLocks("structure,direction,proportions,content"));
    widget.InitializeMultiWidget();

    const std::pair<const char*, unsigned int> layouts[] = {
      { "MxNDemoLayout.json", 2 },
      { "MxNDemoLayoutAlt.json", 3 }
    };
    for (const auto& [fileName, cellCount] : layouts)
    {
      const auto path = resources.filePath(QString::fromUtf8(fileName)).toStdString();
      std::ifstream file(path);
      CPPUNIT_ASSERT_MESSAGE("Example layout must exist: " + path, file.good());

      const auto document = nlohmann::json::parse(file);
      CPPUNIT_ASSERT_NO_THROW_MESSAGE("Example layout must apply: " + path, widget.ApplyLayout(document));
      CPPUNIT_ASSERT_EQUAL_MESSAGE(path, cellCount, widget.GetNumberOfRenderWindowWidgets());
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLayoutLock)
