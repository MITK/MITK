/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMitkWorkbenchIntroPart.h"

#include <berryIPerspectiveDescriptor.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryWorkbenchPreferenceConstants.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QmitkHtmlWidget.h>

#include <QColor>
#include <QDir>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
  // The header and tip frame are native widgets. The dark theme sets
  // "QWidget { background-color: #2d2d30 }", which would tint every container
  // in this subtree, so reset all descendants to transparent (they then show
  // the #welcomeContent background) and repaint only the panels via two-ID
  // selectors that outrank both the reset and the theme rule.
  const char* WELCOME_STYLESHEET =
    "#welcomeContent { background-color: #202021; }"
    "#welcomeContent QWidget { background-color: transparent; }"
    "#welcomeContent QLabel { color: #f1f1f1; font-size: 16px; }"
    "QLabel#welcomeH1 { font-size: 32px; font-weight: bold; }"
    "#welcomeContent #tipsBox { background-color: #3f3f46; border-radius: 16px; }"
    "#welcomeContent #nextTipButton { background-color: #3f3f46; border: 2px solid #f1f1f1;"
    " color: #f1f1f1; font-size: 18px; padding: 6px 14px; }"
    "#welcomeContent #nextTipButton:hover { background-color: #f1f1f1; color: #252526; }";

  // Colour of the tip card. The tip HTML view paints the same colour behind its
  // content (SetPageColor) and tips.css uses it as the body background, so the
  // card and the rendered tip blend seamlessly. Keep all three in sync.
  const QColor TIP_CARD_COLOR("#3f3f46");

  // The welcome tips are standalone HTML documents in the plug-in resources.
  // Whatever *.html files exist here become the tips, so one can be added or
  // removed without touching this code (it only needs a matching .qrc entry).
  const QString TIP_DIR = QStringLiteral(":/org.mitk.gui.qt.welcomescreen/tips");

  QStringList FindTipDocuments()
  {
    QStringList tips;
    const QDir dir(TIP_DIR);

    for (const QString& name : dir.entryList({QStringLiteral("*.html")}, QDir::Files, QDir::Name))
      tips.append(dir.filePath(name));

    return tips;
  }

  QString ReadResource(const QString& path)
  {
    QFile file(path);

    if (file.open(QIODevice::ReadOnly))
      return QString::fromUtf8(file.readAll());

    return QString();
  }

  QLabel* CreateLabel(const QString& text, const QString& objectName = QString())
  {
    auto* label = new QLabel(text);
    label->setWordWrap(true);

    if (!objectName.isEmpty())
      label->setObjectName(objectName);

    return label;
  }
}

QmitkMitkWorkbenchIntroPart::QmitkMitkWorkbenchIntroPart()
  : m_TipsBox(nullptr),
    m_TipView(nullptr),
    m_TipFiles(FindTipDocuments()),
    m_CurrentTip(0)
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

  // if workbench is not closing (Just welcome screen closing), open last used perspective
  if (this->GetIntroSite()->GetPage()->GetPerspective()->GetId()
    == "org.mitk.mitkworkbench.perspectives.editor" && !this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
    {
    berry::IPerspectiveDescriptor::Pointer perspective = this->GetIntroSite()->GetWorkbenchWindow()->GetWorkbench()->GetPerspectiveRegistry()->FindPerspectiveWithId("org.mitk.mitkworkbench.perspectives.editor");
    if (perspective)
    {
      this->GetIntroSite()->GetPage()->SetPerspective(perspective);
    }
  }
}

void QmitkMitkWorkbenchIntroPart::CreateQtPartControl(QWidget* parent)
{
  auto* content = new QWidget;
  content->setObjectName("welcomeContent");
  content->setAttribute(Qt::WA_StyledBackground, true);
  content->setStyleSheet(WELCOME_STYLESHEET);

  auto* column = new QWidget;
  column->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  column->setMaximumWidth(1000);

  auto* centering = new QHBoxLayout(content);
  centering->setContentsMargins(16, 16, 16, 16);
  centering->addStretch(1);
  centering->addWidget(column, 1000);
  centering->addStretch(1);

  auto* columnLayout = new QVBoxLayout(column);
  columnLayout->setContentsMargins(0, 0, 0, 0);
  columnLayout->setSpacing(16);

  columnLayout->addWidget(CreateLabel("Welcome to the MITK Workbench!", "welcomeH1"));
  columnLayout->addWidget(CreateLabel(
    "Load and view medical images from all kinds of imaging modalities like X-ray, US, CT, "
    "MRI, and many more. Use our tools to efficiently create segmentations, measure anatomical "
    "structures, inspect image statistics, or register multiple images to each other."));

  // MITK has no themed-link helper and the default anchor colour is a blue that
  // reads poorly on this dark panel, so colour the link text white via an inline
  // span (setting QPalette::Link is overridden by the global theme stylesheet).
  auto* links = CreateLabel(
    "<small>The MITK Workbench is developed at the "
    "<a href=\"https://www.dkfz.de\"><span style=\"color:#f1f1f1;\">&#8618; German Cancer Research Center (DKFZ)</span></a>.<br>"
    "It is based on the free open source "
    "<a href=\"https://www.mitk.org\"><span style=\"color:#f1f1f1;\">&#8618; Medical Imaging Interaction Toolkit (MITK)</span></a>."
    "</small>");
  links->setTextFormat(Qt::RichText);
  links->setOpenExternalLinks(true);
  columnLayout->addWidget(links);

  // The tip content is HTML rendered by litehtml; the surrounding card and the
  // "Next tip" button are native. Fit-to-content lets the tip flow within the
  // page instead of scrolling inside the card.
  m_TipView = new QmitkHtmlWidget;
  m_TipView->SetFitToContent(true);
  m_TipView->SetPageColor(TIP_CARD_COLOR);
  m_TipView->SetResourceHandler([](const QUrl& url) -> QByteArray
  {
    QFile file(QLatin1Char(':') + url.path());

    if (file.open(QIODevice::ReadOnly))
      return file.readAll();

    return QByteArray();
  });

  auto* nextTip = new QPushButton(QString("Next tip ") + QChar(0x00BB));
  nextTip->setObjectName("nextTipButton");
  nextTip->setCursor(Qt::PointingHandCursor);
  connect(nextTip, &QPushButton::clicked, this, [this]()
  {
    this->ShowTip(m_CurrentTip + 1);
  });

  m_TipsBox = new QWidget;
  m_TipsBox->setObjectName("tipsBox");
  m_TipsBox->setAttribute(Qt::WA_StyledBackground, true);

  auto* tipsLayout = new QVBoxLayout(m_TipsBox);
  tipsLayout->setContentsMargins(16, 8, 16, 16);

  auto* buttonRow = new QHBoxLayout;
  buttonRow->addStretch(1);
  buttonRow->addWidget(nextTip);
  tipsLayout->addLayout(buttonRow);
  tipsLayout->addWidget(m_TipView);

  columnLayout->addWidget(m_TipsBox);
  columnLayout->addStretch(1);

  auto* scrollArea = new QScrollArea;
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setWidgetResizable(true);
  scrollArea->setWidget(content);

  auto* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(scrollArea);

  this->ApplyTipsPreference();
}

void QmitkMitkWorkbenchIntroPart::StandbyStateChanged(bool /*standby*/)
{
}

void QmitkMitkWorkbenchIntroPart::SetFocus()
{
}

void QmitkMitkWorkbenchIntroPart::ReloadPage()
{
  if (m_TipsBox != nullptr)
    this->ApplyTipsPreference();
}

void QmitkMitkWorkbenchIntroPart::ApplyTipsPreference()
{
  auto* prefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.qt.extapplicationintro");
  const bool showTips = prefs->GetBool("show tips", true);

  if (showTips && !m_TipFiles.isEmpty())
    this->ShowTip(QRandomGenerator::global()->bounded(m_TipFiles.size()));

  m_TipsBox->setVisible(showTips);
}

void QmitkMitkWorkbenchIntroPart::ShowTip(int index)
{
  const int count = m_TipFiles.size();

  if (count == 0)
    return;

  m_CurrentTip = ((index % count) + count) % count;
  m_TipView->SetHtml(ReadResource(m_TipFiles[m_CurrentTip]),
    QUrl(QStringLiteral("qrc:/org.mitk.gui.qt.welcomescreen/")));
}
