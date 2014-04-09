/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkLineEdit.h"
#include <QApplication>

QmitkLineEdit::QmitkLineEdit(QWidget* parent)
  : QLineEdit(parent)
{
  this->Initialize();
}

QmitkLineEdit::QmitkLineEdit(const QString& defaultText, QWidget* parent)
  : QLineEdit(parent),
    m_DefaultText(defaultText)
{
  this->Initialize();
}

QmitkLineEdit::~QmitkLineEdit()
{
}

void QmitkLineEdit::focusInEvent(QFocusEvent* event)
{
  QLineEdit::focusInEvent(event);
  emit this->FocusChanged(true);
}

void QmitkLineEdit::focusOutEvent(QFocusEvent* event)
{
  QLineEdit::focusOutEvent(event);
  emit this->FocusChanged(false);
}

QString QmitkLineEdit::GetDefaultText() const
{
  return m_DefaultText;
}

void QmitkLineEdit::Initialize()
{
  m_DefaultPalette.setColor(QPalette::Text, QApplication::palette().color(QPalette::Disabled, QPalette::Text));

  this->ShowDefaultText(true);

  connect(this, SIGNAL(FocusChanged(bool)), this, SLOT(OnFocusChanged(bool)));
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(OnTextChanged(const QString&)));
}

void QmitkLineEdit::OnFocusChanged(bool hasFocus)
{
  if (hasFocus)
  {
    if (this->text() == m_DefaultText && this->palette() == m_DefaultPalette)
      this->ShowDefaultText(false);
  }
  else
  {
    if (this->text().isEmpty())
      this->ShowDefaultText(true);
  }
}

void QmitkLineEdit::OnTextChanged(const QString&)
{
  if (this->palette() == m_DefaultPalette)
    this->setPalette(QPalette());
}

void QmitkLineEdit::SetDefaultText(const QString& defaultText)
{
  m_DefaultText = defaultText;
}

void QmitkLineEdit::ShowDefaultText(bool show)
{
  this->blockSignals(true);

  if (show)
  {
    this->setPalette(m_DefaultPalette);
    this->setText(m_DefaultText);
  }
  else
  {
    this->setPalette(QPalette());
    this->clear();
  }

  this->blockSignals(false);
}

