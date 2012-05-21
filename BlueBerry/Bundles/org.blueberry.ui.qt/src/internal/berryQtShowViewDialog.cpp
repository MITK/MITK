/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtShowViewDialog.h"

namespace berry {

QtShowViewDialog::QtShowViewDialog(IViewRegistry*  /*registry*/, QWidget* parent, Qt::WindowFlags f)
: QDialog(parent, f)
{
  m_UserInterface.setupUi(this);
}

int QtShowViewDialog::Open()
{
  int returnCode = this->exec();
  if (returnCode == QDialog::Accepted)
    return IShowViewDialog::OK;
  else return IShowViewDialog::CANCEL;
}

QList<IViewDescriptor::Pointer>
QtShowViewDialog::GetSelection()
{
  QList<IViewDescriptor::Pointer> selected;

  return selected;
}

}
