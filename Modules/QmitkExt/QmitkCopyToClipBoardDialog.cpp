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

#include "QmitkCopyToClipBoardDialog.h"

#include <qtextedit.h>
#include <qlayout.h>

QmitkCopyToClipBoardDialog::QmitkCopyToClipBoardDialog(const QString& text, QWidget* parent, const char*  /*name*/)
: QDialog(parent)
{
  QBoxLayout * layout = new QVBoxLayout( this );
  //!mmueller
  //QTextEdit* textedit = new QTextEdit(text, "", this);
  //#changed to:
  QTextEdit* textedit = new QTextEdit(this);
  // with setPlainText() line breaks are displayed correctly
  textedit->setPlainText(text);
  //!
  textedit->setReadOnly(true);
  layout->addWidget( textedit );

  QDialog::resize(500,400);
}

QmitkCopyToClipBoardDialog::~QmitkCopyToClipBoardDialog()
{
}

