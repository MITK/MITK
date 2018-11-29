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

#include "QmitkHotkeyLineEdit.h"

#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>

const std::string QmitkHotkeyLineEdit::TOOLTIP = "Press any key (combination)";

QmitkHotkeyLineEdit::QmitkHotkeyLineEdit(QWidget* parent /*= nullptr*/)
  : QLineEdit(parent)
{
  Init();
}

QmitkHotkeyLineEdit::QmitkHotkeyLineEdit(const QKeySequence& qKeySequence, QWidget* parent /*= nullptr*/)
  : QLineEdit(parent)
{
  Init();
  SetKeySequence(qKeySequence);
}

QmitkHotkeyLineEdit::QmitkHotkeyLineEdit(const QString& qKeySequenceAsString, QWidget* parent /*= nullptr*/)
  : QLineEdit(parent)
{
  Init();
  SetKeySequence(qKeySequenceAsString);
}

void QmitkHotkeyLineEdit::Init()
{
  setToolTip(QString::fromStdString(QmitkHotkeyLineEdit::TOOLTIP));
  setReadOnly(true);
  connect(this, &QLineEdit::textChanged, this, &QmitkHotkeyLineEdit::LineEditTextChanged);
}

void QmitkHotkeyLineEdit::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_unknown)
  {
    return;
  }
  else if (event->key() == Qt::Key_Escape)
  {
    m_KeySequence = QKeySequence();
  }
  else
  {
    m_KeySequence = QKeySequence(event->modifiers() + event->key());
  }

  SetKeySequence(m_KeySequence);
}

void QmitkHotkeyLineEdit::SetKeySequence(const QKeySequence& qKeySequence)
{
  setText(qKeySequence.toString());
}

void QmitkHotkeyLineEdit::SetKeySequence(const QString& qKeySequenceAsString)
{
  SetKeySequence(QKeySequence(qKeySequenceAsString));
}

QKeySequence QmitkHotkeyLineEdit::GetKeySequence()
{
  return m_KeySequence;
}

QString QmitkHotkeyLineEdit::GetKeySequenceAsString()
{
  return m_KeySequence.toString();
}

bool QmitkHotkeyLineEdit::Matches(QKeyEvent* event)
{
  QKeySequence keySequence = QKeySequence(event->modifiers() + event->key());

  return keySequence == m_KeySequence;
}

void QmitkHotkeyLineEdit::LineEditTextChanged(const QString& text)
{
  m_KeySequence = QKeySequence(text.toUpper());
}
