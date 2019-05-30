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

#include "berryQtShowViewAction.h"

#include <QApplication>

#include <berryIWorkbenchPage.h>
#include <berryUIException.h>

#include <AutoplanLogging.h>

namespace berry
{

QtShowViewAction::QtShowViewAction(IWorkbenchWindow::Pointer window,
    IViewDescriptor::Pointer desc) :
  QAction(nullptr)
{
  this->setParent(static_cast<QWidget*>(window->GetShell()->GetControl()));

  QString label = QApplication::translate("Plugin Title", desc->GetLabel().toStdString().c_str());
  this->setText(label);

  this->setToolTip(QString(desc->GetLabel().toUtf8().constData()));

  this->setIconVisibleInMenu(true);

  QIcon icon = desc->GetImageDescriptor();
  this->setIcon(icon);

  m_Window = window.GetPointer();
  m_Desc = desc;

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QtShowViewAction::Run()
{
  IWorkbenchPage::Pointer page = m_Window->GetActivePage();
  if (page.IsNotNull())
  {
    try
    {
      page->ShowView(m_Desc->GetId());
    }
    catch (const PartInitException& e)
    {
      BERRY_ERROR << "Error: " << e.what();
    }
  }
}

}
