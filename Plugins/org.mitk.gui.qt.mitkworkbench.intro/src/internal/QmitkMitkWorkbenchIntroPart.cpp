/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMitkWorkbenchIntroPart.h"

#include "QmitkMitkWorkbenchIntroPlugin.h"
#include "QmitkWelcomePalette.h"
#include "QmitkWelcomePersonalizationPage.h"
#include "QmitkWelcomeRecentDataPage.h"
#include "QmitkWelcomeText.h"
#include "QmitkWelcomeTipsPage.h"

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryWorkbenchPreferenceConstants.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkRecentData.h>

#include <QmitkIconTheme.h>

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSizePolicy>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
  // The header, tabs, and cards are native widgets. The dark theme sets
  // "QWidget { background-color: #2d2d30 }", which would tint every container
  // in this subtree, so reset all descendants to transparent (they then show
  // the #welcomeContent background) and repaint only the panels via two-ID
  // selectors that outrank both the reset and the theme rule.
  QString CreateStyleSheet(const QmitkWelcomePalette& palette)
  {
    return QString(
      "#welcomeContent { background-color: %1; }"
      "#welcomeContent QWidget { background-color: transparent; }"
      "#welcomeContent QLabel, #welcomeContent QRadioButton { color: %3; font-size: 16px; }"
      "#welcomeContent #mutedLabel { color: %4; }"
      "QLabel#welcomeH1 { font-size: 32px; font-weight: bold; }"
      "#welcomeContent #cardTitle { font-size: 20px; font-weight: bold; }"
      "#welcomeContent #tipsBox, #welcomeContent #welcomeCard { background-color: %2; border-radius: 16px; }"
      "#welcomeContent #optionList { background-color: %6; border-radius: 8px; }"
      "#welcomeContent #nextTipButton { background-color: %2; border: 2px solid %3;"
      " color: %3; font-size: 18px; padding: 6px 14px; }"
      "#welcomeContent #nextTipButton:hover { background-color: %3; color: %2; }"
      "#welcomeContent #clearButton { border: 1px solid %4; border-radius: 4px; color: %4;"
      " font-size: 14px; padding: 2px 10px; }"
      "#welcomeContent #clearButton:hover { border-color: %3; color: %3; }"
      "#welcomeContent #clearButton:disabled { border-color: transparent; }"
      "#welcomeContent QListWidget { font-size: 16px; }"
      "#welcomeContent QTabWidget::pane { border: none; }"
      "#welcomeContent QTabBar::tab { background-color: transparent; border: none;"
      " border-bottom: 3px solid transparent; color: %4; font-size: 18px; padding: 6px 4px; margin-right: 24px; }"
      "#welcomeContent QTabBar::tab:selected { border-bottom-color: %5; color: %3; }"
      "#welcomeContent QTabBar::tab:!selected:hover { color: %3; }")
      .arg(palette.Page.name(), palette.Card.name(), palette.Text.name(), palette.MutedText.name(), palette.Accent.name(), palette.Cell.name());
  }

  // MITK has no themed-link helper and the default anchor color is a blue that
  // reads poorly on the dark panel, so color the link text via an inline span
  // (setting QPalette::Link is overridden by the global theme style sheet).
  QString CreateLinksText(const QmitkWelcomePalette& palette)
  {
    return QmitkWelcomeTextWithLineHeight(QString(
      "<small>The MITK Workbench is developed at the "
      "<a href=\"https://www.dkfz.de\"><span style=\"color:%1;\">&#8618; German Cancer Research Center (DKFZ)</span></a>.<br>"
      "It is based on the free open source "
      "<a href=\"https://www.mitk.org\"><span style=\"color:%1;\">&#8618; Medical Imaging Interaction Toolkit (MITK)</span></a>."
      "</small>")
      .arg(palette.Text.name()));
  }

  QLabel* CreateLabel(const QString& text, const QString& objectName = QString())
  {
    auto* label = new QLabel(text);
    label->setWordWrap(true);

    if (!objectName.isEmpty())
      label->setObjectName(objectName);

    return label;
  }

  // The tips that a few early starts bring up, in an order in which they build
  // on each other. Other starts show no particular tip.
  QString GetTipOfStart(int startCount)
  {
    switch (startCount)
    {
      case 2:
        return "mouse-navigation";

      case 5:
        return "image-contrast";

      case 10:
        return "view-alignment";

      default:
        return QString();
    }
  }

  bool HasRecentData()
  {
    return !mitk::RecentData::Get(mitk::RecentData::Kind::Project).isEmpty()
      || !mitk::RecentData::Get(mitk::RecentData::Kind::File).isEmpty();
  }
}

QmitkMitkWorkbenchIntroPart::QmitkMitkWorkbenchIntroPart()
  : m_Content(nullptr),
    m_Links(nullptr),
    m_TipsPage(nullptr),
    m_RecentDataPage(nullptr)
{
  auto* workbenchPrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
  workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
  workbenchPrefs->Flush();
}

QmitkMitkWorkbenchIntroPart::~QmitkMitkWorkbenchIntroPart()
{
  // if the workbench is not closing (that means, welcome screen was closed explicitly), set "Show_intro" false
  bool showIntro = this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing();

  auto* workbenchPrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences();
  workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, showIntro);
  workbenchPrefs->Flush();
}

void QmitkMitkWorkbenchIntroPart::CreateQtPartControl(QWidget* parent)
{
  const auto palette = QmitkWelcomePalette::GetCurrent();

  m_Content = new QWidget;
  m_Content->setObjectName("welcomeContent");
  m_Content->setAttribute(Qt::WA_StyledBackground, true);

  auto* column = new QWidget;
  column->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  column->setMaximumWidth(1000);

  auto* centering = new QHBoxLayout(m_Content);
  centering->setContentsMargins(16, 16, 16, 16);
  centering->addStretch(1);
  centering->addWidget(column, 1000);
  centering->addStretch(1);

  auto* columnLayout = new QVBoxLayout(column);
  columnLayout->setContentsMargins(0, 0, 0, 0);
  columnLayout->setSpacing(16);

  columnLayout->addWidget(CreateLabel("Welcome to the MITK Workbench!", "welcomeH1"));
  auto* description = CreateLabel(QmitkWelcomeTextWithLineHeight(
    "Load and view medical images from all kinds of imaging modalities like X-ray, US, CT, "
    "MRI, and many more. Use our tools to efficiently create segmentations, measure anatomical "
    "structures, inspect image statistics, or register multiple images to each other."));
  description->setTextFormat(Qt::RichText);
  columnLayout->addWidget(description);

  m_Links = CreateLabel(QString());
  m_Links->setTextFormat(Qt::RichText);
  m_Links->setOpenExternalLinks(true);
  columnLayout->addWidget(m_Links);

  m_TipsPage = new QmitkWelcomeTipsPage(palette);
  auto* personalizationPage = new QmitkWelcomePersonalizationPage;
  m_RecentDataPage = new QmitkWelcomeRecentDataPage(this->GetIntroSite()->GetWorkbenchWindow(), palette);

  // The size matches the aspect ratio of the light bulb, which is the only tab icon.
  auto* tabs = new QTabWidget;
  tabs->setIconSize(QSize(14, 20));

  for (auto* page : std::initializer_list<QWidget*>{ m_TipsPage, personalizationPage, m_RecentDataPage })
    page->setContentsMargins(0, 16, 0, 0);

  // The tips are rarely shown first, so their tab gets an eye-catching icon.
  tabs->addTab(m_TipsPage, QmitkIconTheme::GetIcon(QStringLiteral(":/org.mitk.gui.qt.welcomescreen/img/tips/lightbulb.svg")), "Tips");
  tabs->addTab(personalizationPage, "Personalization");
  tabs->addTab(m_RecentDataPage, "Recent data");

  // New users see the settings first, everyone else can pick up their work.
  // A few early starts bring up the tips instead.
  const auto tipOfStart = GetTipOfStart(QmitkMitkWorkbenchIntroPlugin::GetDefault()->GetStartCount());

  if (!tipOfStart.isEmpty())
  {
    m_TipsPage->SelectTip(tipOfStart);
    tabs->setCurrentWidget(m_TipsPage);
  }
  else if (HasRecentData())
  {
    tabs->setCurrentWidget(m_RecentDataPage);
  }
  else
  {
    tabs->setCurrentWidget(personalizationPage);
  }

  columnLayout->addWidget(tabs);
  columnLayout->addStretch(1);

  // Only kicks in for windows too small to show everything at once.
  auto* scrollArea = new QScrollArea;
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setWidgetResizable(true);
  scrollArea->setWidget(m_Content);

  auto* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(scrollArea);

  m_Content->setStyleSheet(CreateStyleSheet(palette));
  m_Links->setText(CreateLinksText(palette));

  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, &QmitkMitkWorkbenchIntroPart::ApplyTheme);
}

void QmitkMitkWorkbenchIntroPart::StandbyStateChanged(bool /*standby*/)
{
}

void QmitkMitkWorkbenchIntroPart::SetFocus()
{
}

void QmitkMitkWorkbenchIntroPart::ApplyTheme()
{
  if (m_Content.isNull())
    return;

  const auto palette = QmitkWelcomePalette::GetCurrent();

  m_Content->setStyleSheet(CreateStyleSheet(palette));
  m_Links->setText(CreateLinksText(palette));
  m_TipsPage->SetPalette(palette);
  m_RecentDataPage->SetPalette(palette);
}
