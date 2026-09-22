/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkButtonOverlayWidget.h>

#include <QVBoxLayout>
#include <QApplication>

QmitkButtonOverlayWidget::QmitkButtonOverlayWidget(QWidget* parent)
  : QmitkOverlayWidget(parent)
{
  // The message wraps, and is given the width to wrap into: an overlay explains
  // why what is underneath it cannot be used, which is a sentence, and a
  // sentence laid out at its single-line width is simply cut off by a host that
  // is narrower than it. Callers may still break a line themselves where they
  // want one; wrapping only decides what happens to the rest.
  m_MessageLabel = new QLabel(this);
  m_MessageLabel->setWordWrap(true);
  m_MessageLabel->setAlignment(Qt::AlignCenter);
  QSizePolicy messagePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  messagePolicy.setHeightForWidth(true);
  m_MessageLabel->setSizePolicy(messagePolicy);

  m_PushButton = new QPushButton(this);
  connect(m_PushButton, &QPushButton::clicked,
    this, &QmitkButtonOverlayWidget::Clicked);
  m_PushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  // No alignment on the layout or on the label: either one would place them at
  // their own size hint instead of the overlay's width, and the label would go
  // on asking for its whole text in one line however narrow the overlay got.
  // The stretches above and below still centre the pair vertically, and the
  // button keeps its alignment because it should stay its own width.
  auto* layout = new QVBoxLayout(this);
  layout->addStretch();
  layout->addWidget(m_MessageLabel);
  layout->addWidget(m_PushButton, 0, Qt::AlignCenter);
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
