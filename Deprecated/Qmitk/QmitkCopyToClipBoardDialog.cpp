/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkCopyToClipBoardDialog.h"

#include <qtextedit.h>
#include <qlayout.h>

QmitkCopyToClipBoardDialog::QmitkCopyToClipBoardDialog(const QString& text, QWidget* parent, const char* name)
: QDialog(parent, name)
{
  QBoxLayout * layout = new QVBoxLayout( this );
  QTextEdit* textedit = new QTextEdit(text, "", this);
  textedit->setReadOnly(true);
  layout->addWidget( textedit );

  QDialog::resize(500,400);
}

QmitkCopyToClipBoardDialog::~QmitkCopyToClipBoardDialog()
{
}

