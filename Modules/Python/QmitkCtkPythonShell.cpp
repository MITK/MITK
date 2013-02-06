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
#include "mitkPythonService.h"
#include <mitkModuleContext.h>
#include <usServiceReference.h>
#include <mitkGetModuleContext.h>

struct QmitkCtkPythonShellData
{
    mitk::PythonService* m_PythonService;
};

QmitkCtkPythonShell::QmitkCtkPythonShell(QWidget* parent)
    : ctkPythonConsole(parent), d( new QmitkCtkPythonShellData )
{
    mitk::ModuleContext* context = mitk::GetModuleContext();
    mitk::ServiceReference serviceRef = context->GetServiceReference<mitk::IPythonService>();
    d->m_PythonService = dynamic_cast<mitk::PythonService*> ( context->GetService<mitk::IPythonService>(serviceRef) );

    assert( d->m_PythonService );
    this->initialize( d->m_PythonService->GetPythonManager() );
}

QmitkCtkPythonShell::~QmitkCtkPythonShell()
{
    delete d;
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
    d->m_PythonService->Execute( urls[i].toString(), mitk::IPythonService::SINGLE_LINE_COMMAND );
  }
}

bool QmitkCtkPythonShell::canInsertFromMimeData( const QMimeData *source ) const
{
  return true;
}

void QmitkCtkPythonShell::executeCommand(const QString& command)
{
  ctkPythonConsole::executeCommand(command);
  d->m_PythonService->NotifyObserver(command);
}
