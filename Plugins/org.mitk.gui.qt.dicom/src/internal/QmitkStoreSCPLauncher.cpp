/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkStoreSCPLauncher.h"
#include <QDir>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <mitklogmacros.h>
//#include <QStringListIterator>

QmitkStoreSCPLauncher::QmitkStoreSCPLauncher(QmitkStoreSCPLauncherBuilder* builder) 
: m_StoreSCP(new QProcess())
{
    connect( m_StoreSCP, SIGNAL(error(QProcess::ProcessError)),this, SLOT(OnProcessError(QProcess::ProcessError)));
    connect( m_StoreSCP, SIGNAL(stateChanged(QProcess::ProcessState)),this, SLOT(OnStateChanged(QProcess::ProcessState)));
    SetArgumentList(builder);
}

QmitkStoreSCPLauncher::~QmitkStoreSCPLauncher()
{
    m_StoreSCP->kill();
    delete m_StoreSCP;
}

void QmitkStoreSCPLauncher::StartStoreSCP()
{
    QString storeSCP;
    storeSCP.append(GetPathToExecutable());
    storeSCP.append(QString("/storescp.exe"));
    m_StoreSCP->start(storeSCP,m_ArgumentList);
}

QString QmitkStoreSCPLauncher::GetPathToExecutable()
{
    QDir root;
    QString currentPath =  root.currentPath();
    currentPath = currentPath.split("MITK").at(0);
    currentPath.append("MITK/DCMTK-install/bin");
    return currentPath;
}

void QmitkStoreSCPLauncher::OnProcessError(QProcess::ProcessError err)
{
	switch(err)
	{
	case QProcess::FailedToStart:
        QMessageBox::information(0,"FailedToStart",QString("FailedToStart:\n").append(m_StoreSCP->errorString()));
		break;
	case QProcess::Crashed:
        QMessageBox::information(0,"Crashed",QString("Crashed:\n").append(m_StoreSCP->errorString()));
		break;
	case QProcess::Timedout:
        QMessageBox::information(0,"Timedout",QString("Timedout:\n").append(m_StoreSCP->errorString()));
		break;
	case QProcess::WriteError:
        QMessageBox::information(0,"WriteError",QString("WriteError:\n").append(m_StoreSCP->errorString()));
		break;
	case QProcess::ReadError:
        QMessageBox::information(0,"ReadError",QString("ReadError:\n").append(m_StoreSCP->errorString()));
		break;
	case QProcess::UnknownError:
        QMessageBox::information(0,"UnknownError",QString("UnknownError:\n").append(m_StoreSCP->errorString()));
		break;
	default:
        QMessageBox::information(0,"default",QString("default:\n").append(m_StoreSCP->errorString()));
		break;
	}
}

void QmitkStoreSCPLauncher::OnStateChanged(QProcess::ProcessState status)
{
	switch(status)
	{
	case QProcess::NotRunning:
        QMessageBox::information(0,"NotRunning",QString("NotRunning:\n").append(m_StoreSCP->errorString()));
		break;
	case QProcess::Starting:
		QMessageBox::information(0,"Starting",QString("Starting"));
		break;
	case QProcess::Running:
		QMessageBox::information(0,"Running",QString("Running"));
		break;
	default:
		QMessageBox::information(0,"default",QString("default:\n").append(m_StoreSCP->errorString()));
		break;
	}
}

void QmitkStoreSCPLauncher::SetArgumentList(QmitkStoreSCPLauncherBuilder* builder)
{
    m_ArgumentList << *builder->GetPort() << QString("-aet") <<*builder->GetAETitle() << *builder->GetTransferSyntax()
        << *builder->GetOtherNetworkOptions() << *builder->GetMode() << QString("-od") << *builder->GetOutputDirectory();
}

QString QmitkStoreSCPLauncher::ArgumentListToQString()
{
    QString argumentString;
    QStringListIterator argumentIterator(m_ArgumentList);
    while(argumentIterator.hasNext())
    {
        argumentString.append(" ");
        argumentString.append(argumentIterator.next());
    }
    return argumentString;
}