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
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QSizePolicy>
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

  // Colour of the tip card; the HTML tip view paints the same colour behind its
  // content so it blends seamlessly into the rounded card.
  const QColor TIP_CARD_COLOR("#3f3f46");

  // Style sheet for the HTML tip documents. Mirrors the look of the previous
  // web-based welcome page, which flowed the images and the mouse-button grid
  // more naturally than a widget layout could.
  const char* TIP_STYLE =
    "body { background-color: #3f3f46; color: #f1f1f1;"
    " font-family: \"Roboto\", \"Open Sans\", sans-serif; font-size: 16px;"
    " line-height: 1.25; margin: 0; }"
    "h2 { font-size: 24px; margin: 0 0 8px 0; }"
    "h3 { font-size: 18px; text-align: center; margin: 4px 0; }"
    "p { margin: 8px 0; }"
    ".tip-content { overflow: hidden; }"
    ".center { display: block; margin: 0 auto; }"
    ".cell { background-color: #2d2d30; padding: 8px 16px; vertical-align: top; }"
    ".float-left { float: left; margin-right: 32px; }";

  const char* TIP_BODIES[] =
  {
    "<h2>Mouse navigation</h2>"
    "<div class=\"tip-content\">"
    "<img class=\"center\" style=\"width: 50%\" alt=\"Crosshair\" src=\"img/mousenavigation/crosshair.svg\">"
    "<p>Your images are typically shown from three different view directions at once: "
    "axial, sagittal, and coronal. Together, the view planes resemble the shape of a "
    "three-dimensional crosshair slicing through your image data. Use the mouse to "
    "navigate in these views.</p>"
    "<table cellspacing=\"16\" cellpadding=\"0\" width=\"100%\" style=\"table-layout: fixed;\"><tr>"
    "<td class=\"cell\"><h3>Focus</h3>"
    "<img style=\"width: 100%\" alt=\"Left mouse button\" src=\"img/mousenavigation/click.svg\">"
    "<p>Use the left mouse button to focus all views on the clicked position.</p></td>"
    "<td class=\"cell\"><h3>Zoom</h3>"
    "<img style=\"width: 100%\" alt=\"Right mouse button\" src=\"img/mousenavigation/zoom.svg\">"
    "<p>Press and hold the right mouse button and move the mouse up and down to zoom in and out.</p></td>"
    "<td class=\"cell\"><h3>Pan</h3>"
    "<img style=\"width: 100%\" alt=\"Middle mouse button\" src=\"img/mousenavigation/pan.svg\">"
    "<p>Press and hold the middle mouse button and move the mouse to pan around.</p></td>"
    "<td class=\"cell\"><h3>Scroll</h3>"
    "<img style=\"width: 100%\" alt=\"Scroll wheel\" src=\"img/mousenavigation/scroll.svg\">"
    "<p>Use the scroll wheel to scroll through slices along the view's direction.</p></td>"
    "</tr></table>"
    "</div>",

    "<h2>Image contrast</h2>"
    "<div class=\"tip-content\">"
    "<img class=\"float-left\" style=\"width: 40%\" alt=\"Standard display with level window\" src=\"img/levelwindow/standarddisplay.svg\">"
    "<p>For images, a vertical scale is shown right next to the views. "
    "It is called the <b>level window</b>.</p>"
    "<p>The values shown in the level window represent the pixel intensities of an image. "
    "The blue bar defines the range of pixel intensities that is mapped onto the whole "
    "range of displayable pixel brightness. Hence, a shorter range of pixel intensities "
    "results in higher displayed image contrast.</p>"
    "<p>Grab the blue bar to move it up and down.<br>"
    "Grab the tips of the blue bar to change its size.</p>"
    "<p>Right-click on the level window for many more options.</p>"
    "</div>"
  };

  const int TIP_COUNT = static_cast<int>(sizeof(TIP_BODIES) / sizeof(TIP_BODIES[0]));

  QString BuildTipDocument(const char* body)
  {
    return QStringLiteral("<html><head><style>%1</style></head><body>%2</body></html>")
      .arg(QLatin1String(TIP_STYLE), QLatin1String(body));
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

  if (showTips)
    this->ShowTip(QRandomGenerator::global()->bounded(TIP_COUNT));

  m_TipsBox->setVisible(showTips);
}

void QmitkMitkWorkbenchIntroPart::ShowTip(int index)
{
  if (TIP_COUNT == 0)
    return;

  m_CurrentTip = ((index % TIP_COUNT) + TIP_COUNT) % TIP_COUNT;
  m_TipView->SetHtml(BuildTipDocument(TIP_BODIES[m_CurrentTip]),
    QUrl(QStringLiteral("qrc:/org.mitk.gui.qt.welcomescreen/")));
}
