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

#include "QmitkCtkPythonShell.h"

#include <ctkAbstractPythonManager.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

struct QmitkCtkPythonShellData
{
    ctkAbstractPythonManager *m_PythonManager;
};

QmitkCtkPythonShell::QmitkCtkPythonShell(QWidget* parent)
    : ctkPythonConsole(parent), d( new QmitkCtkPythonShellData )
{
    d->m_PythonManager = 0;
}

QmitkCtkPythonShell::~QmitkCtkPythonShell()
{
    delete d;
}

void QmitkCtkPythonShell::SetPythonManager(ctkAbstractPythonManager *_PythonManager)
{
    d->m_PythonManager = _PythonManager;
    this->initialize( _PythonManager );
}

void QmitkCtkPythonShell::dragEnterEvent(QDragEnterEvent *event)
{
  event->accept();
}
void QmitkCtkPythonShell::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();
  for(int i = 0; i < urls.size(); i++)
  {
    d->m_PythonManager->executeString(urls[i].toString());
  }
}

bool QmitkCtkPythonShell::canInsertFromMimeData( const QMimeData *source ) const
{
  return true;
}

void QmitkCtkPythonShell::executeCommand(const QString& command)
{
  emit executeCommandSignal(command);
  ctkPythonConsole::executeCommand(command);

  emit newCommandExecuted();
}
