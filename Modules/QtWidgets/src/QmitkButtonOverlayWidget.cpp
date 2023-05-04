/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkButtonOverlayWidget.h"

#include <QmitkStyleManager.h>

#include <QVBoxLayout>

QmitkButtonOverlayWidget::QmitkButtonOverlayWidget(QWidget* parent)
  : QmitkOverlayWidget(parent)
{
  m_MessageLabel = new QLabel(this);
  m_MessageLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  m_PushButton = new QPushButton(this);
  connect(m_PushButton, &QPushButton::clicked,
    this, &QmitkButtonOverlayWidget::Clicked);
  m_PushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto* layout = new QVBoxLayout(this);
  layout->setAlignment(Qt::AlignCenter);
  layout->addStretch();
  layout->addWidget(m_MessageLabel);
  layout->addWidget(m_PushButton);
  layout->addStretch();

  this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  this->setAttribute(Qt::WA_NoMousePropagation);
}

QmitkButtonOverlayWidget::~QmitkButtonOverlayWidget()
{
}

QString QmitkButtonOverlayWidget::GetOverlayText() const
{
  return m_MessageLabel->text();
}

void QmitkButtonOverlayWidget::SetOverlayText(const QString& text)
{
  m_MessageLabel->setText(text);
}

QString QmitkButtonOverlayWidget::GetButtonText() const
{
  return m_PushButton->text();
}

void QmitkButtonOverlayWidget::SetButtonText(const QString& text)
{
  m_PushButton->setText(text);
}

QIcon QmitkButtonOverlayWidget::GetButtonIcon() const
{
  return m_PushButton->icon();
}

void QmitkButtonOverlayWidget::SetButtonIcon(const QIcon& icon)
{
  m_PushButton->setIcon(icon);
}
