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

QtShowViewDialog::QtShowViewDialog(const IWorkbenchWindow* window, IViewRegistry* registry,
                                   QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , m_Window(window)
  , m_ViewReg(registry)
{
  m_UserInterface.setupUi(this);
}


QList<IViewDescriptor::Pointer>
QtShowViewDialog::GetSelection() const
{
  QList<IViewDescriptor::Pointer> selected;

  return selected;
}

}
