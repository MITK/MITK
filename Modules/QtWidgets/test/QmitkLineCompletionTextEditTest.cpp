/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkLineCompletionTextEdit.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <memory>

class QmitkLineCompletionTextEditTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkLineCompletionTextEditTestSuite);
  MITK_TEST(TestSuggestionsStartingWithLineComeFirst);
  MITK_TEST(TestSuggestionsIgnoreCase);
  MITK_TEST(TestNoSuggestionsForShortOrUnknownLines);
  MITK_TEST(TestChoosingSuggestionReplacesOnlyCurrentLine);
  MITK_TEST(TestTabChoosesSuggestionAndKeepsFocus);
  MITK_TEST(TestMovingAwayFromLineEndHidesSuggestions);
  MITK_TEST(TestEscapeHidesSuggestions);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    EnsureQApplication();

    // A second widget that Tab can move the focus to. A text control, since
    // whether buttons take the focus by Tab depends on the platform.
    m_Window = std::make_unique<QWidget>();
    auto* layout = new QVBoxLayout(m_Window.get());

    m_Edit = new QmitkLineCompletionTextEdit(m_Window.get());
    m_Edit->SetCompletions({ "left kidney", "liver", "right kidney", "caudate lobe of liver" });
    layout->addWidget(m_Edit);

    m_OtherEdit = new QLineEdit(m_Window.get());
    layout->addWidget(m_OtherEdit);

    m_Window->show();
    m_Edit->setFocus();
  }

  void tearDown() override
  {
    m_Window.reset();
  }

  void TestSuggestionsStartingWithLineComeFirst()
  {
    this->Type("liv");

    const QStringList expected = { "liver", "caudate lobe of liver" };
    CPPUNIT_ASSERT_MESSAGE("Completions that start with the line should precede those that contain it",
                           this->Suggestions() == expected);
  }

  void TestSuggestionsIgnoreCase()
  {
    this->Type("KIDNEY");

    const QStringList expected = { "left kidney", "right kidney" };
    CPPUNIT_ASSERT_MESSAGE("Matching should ignore case", this->Suggestions() == expected);
  }

  void TestNoSuggestionsForShortOrUnknownLines()
  {
    this->Type("l");
    CPPUNIT_ASSERT_MESSAGE("A single character should not be completed", !this->Popup()->isVisible());

    this->Type("x");
    CPPUNIT_ASSERT_MESSAGE("A line without completions should show no suggestions", !this->Popup()->isVisible());
  }

  void TestChoosingSuggestionReplacesOnlyCurrentLine()
  {
    this->Type("liver");
    this->Press(this->Popup(), Qt::Key_Escape);
    this->Press(m_Edit, Qt::Key_Return);
    this->Type("cau");

    CPPUNIT_ASSERT_MESSAGE("A matching line should show suggestions", this->Popup()->isVisible());

    // Keys reach the popup first while it is shown, as they do for the user.
    this->Press(this->Popup(), Qt::Key_Return);

    CPPUNIT_ASSERT_EQUAL(std::string("liver\ncaudate lobe of liver"), m_Edit->toPlainText().toStdString());
    CPPUNIT_ASSERT_MESSAGE("Choosing a suggestion should close the list", !this->Popup()->isVisible());
  }

  void TestTabChoosesSuggestionAndKeepsFocus()
  {
    this->Type("cau");
    CPPUNIT_ASSERT(this->Popup()->isVisible());

    this->Press(this->Popup(), Qt::Key_Tab);

    CPPUNIT_ASSERT_EQUAL(std::string("caudate lobe of liver"), m_Edit->toPlainText().toStdString());
    CPPUNIT_ASSERT_MESSAGE("Choosing a suggestion should close the list", !this->Popup()->isVisible());
    CPPUNIT_ASSERT_MESSAGE("Tab should not move the focus while it chooses a suggestion",
                           m_Window->focusWidget() == m_Edit);

    this->Press(m_Edit, Qt::Key_Tab);

    CPPUNIT_ASSERT_MESSAGE("Tab should move the focus while no suggestions are shown",
                           m_Window->focusWidget() == m_OtherEdit);
  }

  void TestMovingAwayFromLineEndHidesSuggestions()
  {
    this->Type("liv");
    CPPUNIT_ASSERT(this->Popup()->isVisible());

    this->Press(m_Edit, Qt::Key_Left);
    CPPUNIT_ASSERT_MESSAGE("Suggestions should only be shown at the end of a line", !this->Popup()->isVisible());
  }

  void TestEscapeHidesSuggestions()
  {
    this->Type("liv");
    this->Press(this->Popup(), Qt::Key_Escape);

    CPPUNIT_ASSERT(!this->Popup()->isVisible());
    CPPUNIT_ASSERT_EQUAL(std::string("liv"), m_Edit->toPlainText().toStdString());
  }

private:
  void Type(const QString& text)
  {
    for (const auto character : text)
    {
      QKeyEvent press(QEvent::KeyPress, character.toUpper().unicode(), Qt::NoModifier, QString(character));
      QApplication::sendEvent(m_Edit, &press);
    }
  }

  void Press(QWidget* receiver, int key)
  {
    QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier);
    QApplication::sendEvent(receiver, &press);
  }

  QAbstractItemView* Popup() const
  {
    return m_Edit->findChild<QCompleter*>()->popup();
  }

  QStringList Suggestions() const
  {
    QStringList suggestions;

    if (!this->Popup()->isVisible())
      return suggestions;

    const auto* model = m_Edit->findChild<QCompleter*>()->completionModel();

    for (int row = 0; row < model->rowCount(); ++row)
      suggestions << model->index(row, 0).data().toString();

    return suggestions;
  }

  std::unique_ptr<QWidget> m_Window;
  QmitkLineCompletionTextEdit* m_Edit = nullptr;
  QLineEdit* m_OtherEdit = nullptr;
};

MITK_TEST_SUITE_REGISTRATION(QmitkLineCompletionTextEdit)
