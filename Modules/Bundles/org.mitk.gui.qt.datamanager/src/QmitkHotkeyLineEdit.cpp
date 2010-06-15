/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-07-07 16:57:15 +0200 (Di, 07 Jul 2009) $
 Version:   $Revision: 18019 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkHotkeyLineEdit.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QKeyEvent>

const QString QmitkHotkeyLineEdit::TOOLTIP = "Press any key (combination)";

QmitkHotkeyLineEdit::QmitkHotkeyLineEdit( QWidget* parent )
: QLineEdit(parent)
{
  this->setToolTip(QmitkHotkeyLineEdit::TOOLTIP);
  //this->setReadOnly(true);
}

QmitkHotkeyLineEdit::QmitkHotkeyLineEdit( const QKeySequence& _QKeySequence, QWidget* parent)
: QLineEdit(parent)
{
  //this->setReadOnly(true);
  this->setToolTip(QmitkHotkeyLineEdit::TOOLTIP);
  this->SetKeySequence(_QKeySequence);
}

QmitkHotkeyLineEdit::QmitkHotkeyLineEdit( const QString& _QKeySequenceAsString, QWidget* parent)
: QLineEdit(parent)
{
  this->setToolTip(QmitkHotkeyLineEdit::TOOLTIP);
  //this->setReadOnly(true);
  this->SetKeySequence(_QKeySequenceAsString);
}

void QmitkHotkeyLineEdit::keyPressEvent( QKeyEvent * event )
{
  if(event->key() == Qt::Key_unknown)
    return;

  m_KeySequence = QKeySequence(event->modifiers(), event->key());
  // if no modifier was pressed the sequence is now empty
  if(event->modifiers() == Qt::NoModifier)
    m_KeySequence = QKeySequence(event->key());

  this->SetKeySequence(m_KeySequence);
}

void QmitkHotkeyLineEdit::SetKeySequence( const QKeySequence& _QKeySequence)
{
  m_KeySequence = _QKeySequence;
  this->setText(m_KeySequence.toString());
}

void QmitkHotkeyLineEdit::SetKeySequence( const QString& _QKeySequenceAsString )
{
  this->SetKeySequence(QKeySequence(_QKeySequenceAsString));
}

QKeySequence QmitkHotkeyLineEdit::GetKeySequence()
{
  return m_KeySequence;
}

QString QmitkHotkeyLineEdit::GetKeySequenceAsString()
{
  return m_KeySequence.toString();
}