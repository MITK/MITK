/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

std::vector<IViewDescriptor::Pointer>
QtShowViewDialog::GetSelection()
{
  std::vector<IViewDescriptor::Pointer> selected;

  return selected;
}

}
