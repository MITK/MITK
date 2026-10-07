/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWelcomeContributePage.h"

#include "QmitkWelcomeCard.h"
#include "QmitkWelcomeText.h"

#include <QmitkFeedback.h>
#include <QmitkIconTheme.h>

#ifdef MITK_HAS_CRASHHANDLING
#include <QmitkCrashDumpManagerDialog.h>
#endif

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
  constexpr int PictogramSize = 40;
}

QmitkWelcomeContributePage::QmitkWelcomeContributePage(const QmitkWelcomePalette& palette, QWidget* parent)
  : QWidget(parent),
    m_Palette(palette)
{
  std::vector<QWidget*> cards;

  cards.push_back(this->CreateCard("feedback.svg", "Report a bug or an idea",
    "Something not working, or a feature missing? Tell us in a few clicks. "
    "You see everything before it leaves your computer.",
    this->CreateButton("Send Feedback...", &QmitkWelcomeContributePage::OnSendFeedback)));

#ifdef MITK_HAS_CRASHHANDLING
  cards.push_back(this->CreateCard("diagnostics.svg", "Share crash data",
    "If MITK crashed or froze, the saved diagnostic data shows us what went wrong. "
    "It stays on your computer until you decide to share it.",
    this->CreateButton("Diagnostic Data...", &QmitkWelcomeContributePage::OnShowDiagnosticData)));
#endif

  cards.push_back(this->CreateCard("code.svg", "Contribute code or docs",
    "MITK is open source. Fix a bug, add a feature, or improve the documentation with us.",
    this->CreateLinks({
      { "https://github.com/MITK/MITK/blob/master/CONTRIBUTING.md", "Contribution guide" },
      { "https://docs.mitk.org/nightly/DeveloperManualPortal.html", "Developer manual" } })));

  // GitHub renders the repository's CITATION.cff as "Cite this repository".
  cards.push_back(this->CreateCard("cite.svg", "Cite MITK",
    "Does MITK support your research? A citation helps us keep it going.",
    this->CreateLinks({
      { "https://doi.org/10.1007/s11548-013-0840-8", "Paper (IJCARS 2013)" },
      { "https://github.com/MITK/MITK", "Software citation" } })));

  auto* grid = new QGridLayout;
  grid->setSpacing(16);
  grid->setColumnStretch(0, 1);
  grid->setColumnStretch(1, 1);

  for (int i = 0; i < static_cast<int>(cards.size()); ++i)
    grid->addWidget(cards[i], i / 2, i % 2);

  // The tabs are as high as their highest page, which the cards would fill.
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(16);
  layout->addWidget(QmitkWelcomeCreateDetailText("MITK grows with its community. Pick what suits you."));
  layout->addLayout(grid);
  layout->addStretch(1);
}

QmitkWelcomeContributePage::~QmitkWelcomeContributePage() = default;

void QmitkWelcomeContributePage::SetPalette(const QmitkWelcomePalette& palette)
{
  m_Palette = palette;

  for (const auto& [label, path] : m_Pictograms)
  {
    label->setPixmap(QmitkIconTheme::GetIcon(path).pixmap(QSize(PictogramSize, PictogramSize), this->devicePixelRatioF()));
  }

  for (const auto& [label, links] : m_Links)
    label->setText(this->CreateLinksText(links));
}

QWidget* QmitkWelcomeContributePage::CreateCard(const QString& pictogram, const QString& title, const QString& text, QWidget* action)
{
  const auto path = ":/org.mitk.gui.qt.welcomescreen/img/contribute/" + pictogram;

  auto* pictogramLabel = new QLabel;
  pictogramLabel->setFixedSize(PictogramSize, PictogramSize);
  pictogramLabel->setPixmap(QmitkIconTheme::GetIcon(path).pixmap(QSize(PictogramSize, PictogramSize), this->devicePixelRatioF()));
  m_Pictograms.emplace_back(pictogramLabel, path);

  auto [card, layout] = QmitkWelcomeCreateCard(title, pictogramLabel);
  layout->addWidget(QmitkWelcomeCreateText(text));
  layout->addStretch(1);
  layout->addWidget(action);

  return card;
}

QWidget* QmitkWelcomeContributePage::CreateButton(const QString& text, void (QmitkWelcomeContributePage::*onClicked)())
{
  auto* button = new QPushButton(text);
  button->setObjectName("cardButton");
  button->setCursor(Qt::PointingHandCursor);
  connect(button, &QPushButton::clicked, this, onClicked);

  auto* row = new QWidget;
  auto* layout = new QHBoxLayout(row);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(button);
  layout->addStretch(1);

  return row;
}

QWidget* QmitkWelcomeContributePage::CreateLinks(const std::vector<Link>& links)
{
  auto* label = QmitkWelcomeCreateText(this->CreateLinksText(links));
  label->setOpenExternalLinks(true);
  QmitkWelcomeShowLinkTargets(label);
  m_Links.emplace_back(label, links);

  return label;
}

QString QmitkWelcomeContributePage::CreateLinksText(const std::vector<Link>& links) const
{
  QStringList anchors;

  // Colored by an inline span like the links below the welcome heading: the
  // theme style sheet overrides QPalette::Link.
  for (const auto& link : links)
  {
    anchors << QString("<a href=\"%1\"><span style=\"color:%2;\">&#8618; %3</span></a>")
      .arg(link.Url, m_Palette.Text.name(), link.Text);
  }

  return QmitkWelcomeTextWithLineHeight(anchors.join("&nbsp;&nbsp;&nbsp;"));
}

void QmitkWelcomeContributePage::OnSendFeedback()
{
  QmitkFeedback::OpenDialog(this->window());
}

void QmitkWelcomeContributePage::OnShowDiagnosticData()
{
#ifdef MITK_HAS_CRASHHANDLING
  QmitkCrashDumpManagerDialog::ShowManager(this->window());
#endif
}
