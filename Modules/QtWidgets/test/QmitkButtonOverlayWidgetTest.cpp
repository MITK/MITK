/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkButtonOverlayWidget.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QLabel>
#include <QLayout>
#include <QPushButton>

/**
 * An overlay explains why what is underneath it cannot be used, which is a
 * sentence rather than a word. These cover what that costs it: the message has
 * to survive a host narrower than the sentence, instead of being laid out at
 * its single-line width and cut off by the edge.
 */
class QmitkButtonOverlayWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkButtonOverlayWidgetTestSuite);

  MITK_TEST(Message_StaysInsideANarrowOverlay);
  MITK_TEST(Message_WrapsInsteadOfGrowingWider);
  MITK_TEST(Button_KeepsItsOwnWidthAndStaysClickable);

  CPPUNIT_TEST_SUITE_END();

  static const char* LongMessage()
  {
    return "<b>No display is open.</b><br/>This view configures the window "
           "arrangement and the synchronization of a display, and has nothing "
           "to act on until one is open.";
  }

  /** The overlay laid out at 'width', with a message too long for one line. */
  static void Realize(QmitkButtonOverlayWidget& overlay, int width)
  {
    overlay.SetOverlayText(QString::fromLatin1(LongMessage()));
    overlay.SetButtonText(QStringLiteral("Open display"));
    overlay.resize(width, 400);
    overlay.layout()->activate();
  }

  static QLabel* MessageOf(const QmitkButtonOverlayWidget& overlay)
  {
    const auto labels = overlay.findChildren<QLabel*>();
    CPPUNIT_ASSERT_MESSAGE("The overlay carries a message label", !labels.isEmpty());
    return labels.first();
  }

public:

  void setUp() override { EnsureQApplication(); }

  void Message_StaysInsideANarrowOverlay()
  {
    // The failure this guards is not subtle: laid out at its single-line width,
    // the label simply runs past the overlay's edge and the tail is unreadable.
    for (const int width : { 600, 320, 200, 140 })
    {
      QmitkButtonOverlayWidget overlay;
      Realize(overlay, width);
      const auto* message = MessageOf(overlay);
      CPPUNIT_ASSERT_MESSAGE("The message never reaches past the overlay's edge",
                             message->geometry().right() <= overlay.rect().right());
      CPPUNIT_ASSERT_MESSAGE("nor past its left edge",
                             message->geometry().left() >= overlay.rect().left());
    }
  }

  void Message_WrapsInsteadOfGrowingWider()
  {
    QmitkButtonOverlayWidget wide;
    Realize(wide, 600);
    QmitkButtonOverlayWidget narrow;
    Realize(narrow, 200);

    const auto* wideMessage = MessageOf(wide);
    const auto* narrowMessage = MessageOf(narrow);

    CPPUNIT_ASSERT_MESSAGE("A narrower overlay gets a narrower message",
                           narrowMessage->width() < wideMessage->width());
    // The text it could not fit on a line has to go somewhere, and wrapping is
    // the only place left once the width is fixed.
    CPPUNIT_ASSERT_MESSAGE("which it answers by growing taller, not by clipping",
                           narrowMessage->height() > wideMessage->height());
  }

  void Button_KeepsItsOwnWidthAndStaysClickable()
  {
    // Only the message spans the overlay; the button is still sized to its own
    // text, centred under it.
    QmitkButtonOverlayWidget overlay;
    Realize(overlay, 600);

    const auto buttons = overlay.findChildren<QPushButton*>();
    CPPUNIT_ASSERT_MESSAGE("The overlay carries a button", !buttons.isEmpty());
    auto* button = buttons.first();
    CPPUNIT_ASSERT_MESSAGE("The button keeps its own width", button->width() < overlay.width());

    int clicks = 0;
    QObject::connect(&overlay, &QmitkButtonOverlayWidget::Clicked, [&clicks]() { ++clicks; });
    button->click();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("and still reports its clicks", 1, clicks);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkButtonOverlayWidget)
