/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWelcomeTipsPage.h"

#include <QmitkHtmlWidget.h>
#include <QmitkIconTheme.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRandomGenerator>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
  const QString BASE_URL = QStringLiteral("qrc:/org.mitk.gui.qt.welcomescreen/");

  // Whatever *.html files exist here become the tips, so one can be added or
  // removed without touching this code (it only needs a matching .qrc entry).
  const QString TIP_DIR = QStringLiteral(":/org.mitk.gui.qt.welcomescreen/tips");

  // Not a real resource: the tips link it next to tips.css and the resource
  // handler generates it from the palette of the current theme.
  const QString THEME_STYLE_SHEET = QStringLiteral("/org.mitk.gui.qt.welcomescreen/tips/theme.css");

  QStringList FindTipDocuments()
  {
    QStringList tips;
    const QDir dir(TIP_DIR);

    for (const QString& name : dir.entryList({QStringLiteral("*.html")}, QDir::Files, QDir::Name))
      tips.append(dir.filePath(name));

    return tips;
  }

  QByteArray ReadResource(const QString& path)
  {
    QFile file(path);

    if (file.open(QIODevice::ReadOnly))
      return file.readAll();

    return QByteArray();
  }

  QByteArray CreateThemeStyleSheet(const QmitkWelcomePalette& palette)
  {
    return QString(
      "body { background-color: %1; color: %2; }\n"
      ".cell { background-color: %3; }\n")
      .arg(palette.Card.name(), palette.Text.name(), palette.Cell.name())
      .toUtf8();
  }
}

QmitkWelcomeTipsPage::QmitkWelcomeTipsPage(const QmitkWelcomePalette& palette, QWidget* parent)
  : QWidget(parent),
    m_Palette(palette),
    m_TipView(nullptr),
    m_TipFiles(FindTipDocuments()),
    m_CurrentTip(0)
{
  // The tip content is HTML rendered by litehtml; the surrounding card and the
  // "Next tip" button are native. Fit-to-content lets the tip flow within the
  // page instead of scrolling inside the card.
  m_TipView = new QmitkHtmlWidget;
  m_TipView->SetFitToContent(true);
  m_TipView->SetPageColor(m_Palette.Card);
  m_TipView->SetResourceHandler([this](const QUrl& url) -> QByteArray
  {
    return this->GetResource(url);
  });

  auto* nextTip = new QPushButton(QString("Next tip ") + QChar(0x00BB));
  nextTip->setObjectName("nextTipButton");
  nextTip->setCursor(Qt::PointingHandCursor);
  connect(nextTip, &QPushButton::clicked, this, [this]()
  {
    this->ShowTip(m_CurrentTip + 1);
  });

  auto* tipsBox = new QWidget;
  tipsBox->setObjectName("tipsBox");
  tipsBox->setAttribute(Qt::WA_StyledBackground, true);

  auto* tipsLayout = new QVBoxLayout(tipsBox);
  tipsLayout->setContentsMargins(16, 8, 16, 16);

  auto* buttonRow = new QHBoxLayout;
  buttonRow->addStretch(1);
  buttonRow->addWidget(nextTip);
  tipsLayout->addLayout(buttonRow);
  tipsLayout->addWidget(m_TipView);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(tipsBox);
  layout->addStretch(1);

  if (!m_TipFiles.isEmpty())
    this->ShowTip(QRandomGenerator::global()->bounded(m_TipFiles.size()));
}

void QmitkWelcomeTipsPage::SetPalette(const QmitkWelcomePalette& palette)
{
  m_Palette = palette;
  m_TipView->SetPageColor(m_Palette.Card);

  // Reloading the tip fetches the theme style sheet and illustrations again.
  this->ShowTip(m_CurrentTip);
}

void QmitkWelcomeTipsPage::SelectTip(const QString& name)
{
  for (int i = 0; i < m_TipFiles.size(); ++i)
  {
    if (QFileInfo(m_TipFiles[i]).completeBaseName() == name)
    {
      this->ShowTip(i);
      return;
    }
  }
}

QByteArray QmitkWelcomeTipsPage::GetResource(const QUrl& url) const
{
  const QString path = url.path();

  if (path == THEME_STYLE_SHEET)
    return CreateThemeStyleSheet(m_Palette);

  const auto data = ReadResource(QLatin1Char(':') + path);

  return path.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive)
    ? QmitkIconTheme::GetThemedSVG(data, m_Palette.Text.name())
    : data;
}

void QmitkWelcomeTipsPage::ShowTip(int index)
{
  const int count = m_TipFiles.size();

  if (count == 0)
    return;

  m_CurrentTip = ((index % count) + count) % count;
  m_TipView->SetHtml(QString::fromUtf8(ReadResource(m_TipFiles[m_CurrentTip])), QUrl(BASE_URL));
}
