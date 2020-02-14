/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCopyToClipBoardDialog.h"

#include <qlayout.h>
#include <qtextedit.h>

QmitkCopyToClipBoardDialog::QmitkCopyToClipBoardDialog(const QString &text, QWidget *parent, const char * /*name*/)
  : QDialog(parent)
{
  QBoxLayout *layout = new QVBoxLayout(this);
  //! mmueller
  // QTextEdit* textedit = new QTextEdit(text, "", this);
  //#changed to:
  auto textedit = new QTextEdit(this);
  // with setPlainText() line breaks are displayed correctly
  textedit->setPlainText(text);
  //!
  textedit->setReadOnly(true);
  layout->addWidget(textedit);

  QDialog::resize(500, 400);
}

QmitkCopyToClipBoardDialog::~QmitkCopyToClipBoardDialog()
{
}
