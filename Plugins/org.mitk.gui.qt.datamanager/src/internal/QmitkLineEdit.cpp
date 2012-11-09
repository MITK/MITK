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

QmitkLineEdit::QmitkLineEdit(QWidget *parent)
  : QLineEdit(parent)
{
  initialize();
}

QmitkLineEdit::QmitkLineEdit(const QString &defaultText, QWidget *parent)
  : QLineEdit(parent),
    m_DefaultText(defaultText)
{
  initialize();
}

QmitkLineEdit::~QmitkLineEdit()
{
}

void QmitkLineEdit::initialize()
{
  m_DefaultPalette.setColor(QPalette::Text, QApplication::palette().color(QPalette::Disabled, QPalette::Text));

  showDefaultText(true);

  connect(this, SIGNAL(focusChanged(bool)), this, SLOT(onFocusChanged(bool)));
  connect(this, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
}

QString QmitkLineEdit::defaultText() const
{
  return m_DefaultText;
}

void QmitkLineEdit::setDefaultText(const QString &defaultText)
{
  m_DefaultText = defaultText;
}

void QmitkLineEdit::focusInEvent(QFocusEvent *event)
{
  QLineEdit::focusInEvent(event);
  emit(focusChanged(true));
}

void QmitkLineEdit::focusOutEvent(QFocusEvent *event)
{
  QLineEdit::focusOutEvent(event);
  emit(focusChanged(false));
}

void QmitkLineEdit::onFocusChanged(bool hasFocus)
{
  if (hasFocus)
  {
    if (text() == m_DefaultText && palette() == m_DefaultPalette)
      showDefaultText(false);
  }
  else
  {
    if (text().isEmpty())
      showDefaultText(true);
  }
}

void QmitkLineEdit::onTextChanged(const QString &text)
{
  if (palette() == m_DefaultPalette)
    setPalette(m_Palette);
}

void QmitkLineEdit::showDefaultText(bool show)
{
  blockSignals(true);

  if (show)
  {
    setPalette(m_DefaultPalette);
    setText(m_DefaultText);
  }
  else
  {
    setPalette(m_Palette);
    clear();
  }

  blockSignals(false);
}
