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
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <usGetModuleContext.h>

struct QmitkCtkPythonShellData
{
    mitk::PythonService* m_PythonService;
    us::ServiceReference<mitk::PythonService> m_PythonServiceRef;
};

QmitkCtkPythonShell::QmitkCtkPythonShell(QWidget* parent)
    : ctkPythonConsole(parent), d( new QmitkCtkPythonShellData )
{
  MITK_DEBUG("QmitkCtkPythonShell") << "retrieving  IPythonService";
  us::ModuleContext* context = us::GetModuleContext();
  d->m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
  d->m_PythonService = dynamic_cast<mitk::PythonService*> ( context->GetService<mitk::IPythonService>(d->m_PythonServiceRef) );

  MITK_DEBUG("QmitkCtkPythonShell") << "checking  IPythonService";
  Q_ASSERT( d->m_PythonService );

  MITK_DEBUG("QmitkCtkPythonShell") << "initialize  m_PythonService";
  this->initialize( d->m_PythonService->GetPythonManager() );

  MITK_DEBUG("QmitkCtkPythonShell") << "m_PythonService initialized";
}

QmitkCtkPythonShell::~QmitkCtkPythonShell()
{
  us::ModuleContext* context = us::GetModuleContext();
  context->UngetService( d->m_PythonServiceRef );
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
    d->m_PythonService->Execute( urls[i].toString().toStdString(), mitk::IPythonService::SINGLE_LINE_COMMAND );
  }
}

bool QmitkCtkPythonShell::canInsertFromMimeData( const QMimeData *source ) const
{
  return true;
}

void QmitkCtkPythonShell::executeCommand(const QString& command)
{
  MITK_DEBUG("QmitkCtkPythonShell") << "executing command " << command.toStdString();
  d->m_PythonService->Execute(command.toStdString(),mitk::IPythonService::MULTI_LINE_COMMAND);
  d->m_PythonService->NotifyObserver(command.toStdString());
}

void QmitkCtkPythonShell::Paste(const QString &command)
{
  if( this->isVisible() )
  {
    this->exec( command );
    //this->executeCommand( command );
  }
}
