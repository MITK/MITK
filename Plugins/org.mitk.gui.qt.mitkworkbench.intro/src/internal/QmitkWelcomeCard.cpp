/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWelcomeCard.h"

#include "QmitkWelcomeText.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

std::pair<QWidget*, QVBoxLayout*> QmitkWelcomeCreateCard(const QString& title, QWidget* pictogram)
{
  auto* card = new QWidget;
  card->setObjectName("welcomeCard");
  card->setAttribute(Qt::WA_StyledBackground, true);

  auto* layout = new QVBoxLayout;
  layout->setSpacing(8);

  auto* titleLabel = new QLabel(title);
  titleLabel->setObjectName("cardTitle");
  layout->addWidget(titleLabel);

  if (pictogram != nullptr)
  {
    layout->setContentsMargins(0, 0, 0, 0);

    auto* columns = new QHBoxLayout(card);
    columns->setContentsMargins(16, 12, 16, 16);
    columns->setSpacing(16);
    columns->addWidget(pictogram, 0, Qt::AlignTop);
    columns->addLayout(layout, 1);
  }
  else
  {
    layout->setContentsMargins(16, 12, 16, 16);
    card->setLayout(layout);
  }

  return { card, layout };
}

QLabel* QmitkWelcomeCreateText(const QString& html)
{
  auto* label = new QLabel(QmitkWelcomeTextWithLineHeight(html));
  label->setTextFormat(Qt::RichText);
  label->setWordWrap(true);

  return label;
}

QLabel* QmitkWelcomeCreateDetailText(const QString& html)
{
  auto* label = QmitkWelcomeCreateText(html);
  label->setObjectName("mutedLabel");

  return label;
}
