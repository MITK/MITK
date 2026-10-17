/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"
#include "QmitkTestPopupProbe.h"

#include <QmitkMxNCellOverlay.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkRenderWindowProximity.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkImageGenerator.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QToolButton>

#include <cstdlib>
#include <memory>

/**
 * The window title in the revealed utility strip: between the controls and
 * the barcode, centred on the cell rather than on the room it gets, elided to
 * that room, and gone below the minimum width. The collapsed title, which the
 * overlay paints, sits where the strip's title appears.
 */
class QmitkMxNUtilityStripTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNUtilityStripTestSuite);

  MITK_TEST(Title_ShowsTheNameCentredOnTheCell);
  MITK_TEST(Title_KeepsItsPlaceWhenTheStripReveals);
  MITK_TEST(Title_ElidesToTheRoomAroundTheCentre);
  MITK_TEST(Title_HiddenBelowTheMinimumWidth);
  MITK_TEST(Title_HiddenWhenEmptyOrSwitchedOff);
  MITK_TEST(Title_BarcodeKeepsItsWidth);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();
    auto imageNode = mitk::DataNode::New();
    imageNode->SetName("image");
    imageNode->SetData(mitk::ImageGenerator::GenerateGradientImage<short>(16, 16, 8, 1.0f, 1.0f, 1.0f));
    imageNode->SetIntProperty("layer", 0);
    m_DataStorage->Add(imageNode);

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    this->ShowEditor(800);
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_DataStorage = nullptr;
  }

  void ShowEditor(int width)
  {
    m_Editor->resize(width, 400);
    m_Editor->show();
    QmitkTestPopupProbe::Pump(200);
  }

  QmitkRenderWindowWidget* Cell() const
  {
    return m_Editor->GetRenderWindowWidget(QStringLiteral("mxn__widget0")).get();
  }

  QmitkRenderWindowUtilityWidget* Strip() const
  {
    return this->Cell()->GetUtilityWidget();
  }

  QmitkMxNCellOverlay* Overlay() const
  {
    return this->Cell()->findChild<QmitkMxNCellOverlay*>();
  }

  /** Bring the pointer to the cell's top edge, which reveals the strip. */
  void Reveal() const
  {
    auto* proximity = this->Cell()->findChild<QmitkRenderWindowProximity*>();
    CPPUNIT_ASSERT(nullptr != proximity);
    proximity->HandlePointerMoved(QPoint(this->Cell()->width() / 2, 2));
    QmitkTestPopupProbe::Pump(300);
    CPPUNIT_ASSERT_MESSAGE("Precondition: the strip is revealed", this->Strip()->isVisible());
  }

  /** The strip's title rect in cell coordinates. */
  QRect StripTitleInCell() const
  {
    const QRect rect = this->Strip()->TitleRect();
    return rect.isValid() ? rect.translated(this->Strip()->pos()) : QRect();
  }

  void Title_ShowsTheNameCentredOnTheCell()
  {
    m_Editor->SetCellDisplayName(QStringLiteral("mxn__widget0"), QStringLiteral("Working plane"));
    this->Reveal();

    CPPUNIT_ASSERT_EQUAL(std::string("Working plane"), this->Strip()->TitleText().toStdString());
    const QRect title = this->Strip()->TitleRect();
    CPPUNIT_ASSERT_MESSAGE("The title has a rect", title.isValid());
    const int stripCentre = this->Strip()->width() / 2;
    CPPUNIT_ASSERT_MESSAGE("Centred on the strip, which spans the cell, not on its own room",
                           std::abs(title.center().x() - stripCentre) <= 1);
  }

  void Title_KeepsItsPlaceWhenTheStripReveals()
  {
    m_Editor->SetCellDisplayName(QStringLiteral("mxn__widget0"), QStringLiteral("Overview"));
    QmitkTestPopupProbe::Pump(50);
    const QRect collapsed = this->Overlay()->TitleRect();
    CPPUNIT_ASSERT_MESSAGE("Precondition: the collapsed title is painted", collapsed.isValid());

    this->Reveal();
    CPPUNIT_ASSERT_MESSAGE("Revealed, the overlay hands the title to the strip", !this->Overlay()->TitleRect().isValid());
    const QRect revealed = this->StripTitleInCell();
    CPPUNIT_ASSERT_MESSAGE("The strip shows the title", revealed.isValid());

    CPPUNIT_ASSERT_MESSAGE("The horizontal centre stays (within 1 px)",
                           std::abs(collapsed.center().x() - revealed.center().x()) <= 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The vertical position stays", collapsed.top(), revealed.top());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The size stays", collapsed.size().height(), revealed.size().height());
  }

  void Title_ElidesToTheRoomAroundTheCentre()
  {
    const QString name = QStringLiteral("A window name far too long for the room the strip leaves around its centre, "
                                        "which is bounded by the buttons on the one side and the barcode on the other");
    m_Editor->SetCellDisplayName(QStringLiteral("mxn__widget0"), name);
    this->Reveal();

    const QString shown = this->Strip()->TitleText();
    CPPUNIT_ASSERT_MESSAGE("A long name is shown elided", !shown.isEmpty() && shown.size() < name.size());

    const QRect title = this->Strip()->TitleRect();
    CPPUNIT_ASSERT_MESSAGE("Still centred on the cell", std::abs(title.center().x() - this->Strip()->width() / 2) <= 1);
    for (const auto* control : this->Strip()->findChildren<QWidget*>())
    {
      const bool isControl = nullptr != qobject_cast<const QToolButton*>(control)
                             || nullptr != qobject_cast<const QmitkMxNSyncBarcodeWidget*>(control);
      if (isControl && control->isVisible() && control->parentWidget() == this->Strip())
      {
        CPPUNIT_ASSERT_MESSAGE(std::string("The title leaves the controls and the barcode free: ")
                                 + control->metaObject()->className(),
                               !title.intersects(control->geometry()));
      }
    }
  }

  void Title_HiddenBelowTheMinimumWidth()
  {
    // So narrow that the leading controls reach past the cell's centre.
    this->ShowEditor(180);
    m_Editor->SetCellDisplayName(QStringLiteral("mxn__widget0"), QStringLiteral("Detail"));
    this->Reveal();

    CPPUNIT_ASSERT_MESSAGE("No room around the centre: no title", this->Strip()->TitleText().isEmpty());
    CPPUNIT_ASSERT_MESSAGE("...and no rect", !this->Strip()->TitleRect().isValid());
  }

  void Title_HiddenWhenEmptyOrSwitchedOff()
  {
    this->Reveal();
    CPPUNIT_ASSERT_MESSAGE("An unnamed window has no title", this->Strip()->TitleText().isEmpty());

    m_Editor->SetCellDisplayName(QStringLiteral("mxn__widget0"), QStringLiteral("Edit"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A rename reaches the revealed strip", std::string("Edit"),
                                 this->Strip()->TitleText().toStdString());

    m_Editor->SetWindowTitlesVisible(false);
    CPPUNIT_ASSERT_MESSAGE("Titles switched off", this->Strip()->TitleText().isEmpty());
    m_Editor->SetWindowTitlesVisible(true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("...and on again", std::string("Edit"), this->Strip()->TitleText().toStdString());
  }

  void Title_BarcodeKeepsItsWidth()
  {
    auto* barcode = this->Strip()->findChild<QmitkMxNSyncBarcodeWidget*>();
    CPPUNIT_ASSERT(nullptr != barcode);

    m_Editor->SetCellDisplayName(QStringLiteral("mxn__widget0"),
                                 QStringLiteral("A long window name that wants all the room it can get"));
    this->Reveal();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The title takes only leftover room: the barcode keeps its glyph width",
                                 barcode->sizeHint().width(), barcode->width());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNUtilityStrip)
