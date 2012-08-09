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
#include "QmitkStoreSCPLauncher.h"
#include <QDir>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <mitkLogMacros.h>

#include <fstream>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDir>
#include <QCoreApplication>
#include "org_mitk_gui_qt_dicom_config.h"

QmitkStoreSCPLauncher::QmitkStoreSCPLauncher(QmitkStoreSCPLauncherBuilder* builder) 
: m_StoreSCP(new QProcess())
{
    connect( m_StoreSCP, SIGNAL(error(QProcess::ProcessError)),this, SLOT(OnProcessError(QProcess::ProcessError)));
    connect( m_StoreSCP, SIGNAL(stateChanged(QProcess::ProcessState)),this, SLOT(OnStateChanged(QProcess::ProcessState)));
    SetArgumentList(builder);
}

QmitkStoreSCPLauncher::~QmitkStoreSCPLauncher()
{
    m_StoreSCP->close();
    m_StoreSCP->waitForFinished(1000);
    delete m_StoreSCP;
}

void QmitkStoreSCPLauncher::StartStoreSCP()
{
    FindPathToStoreSCP();
    MITK_INFO << m_PathToStoreSCP.toStdString();
    MITK_INFO << m_ArgumentList[7].toStdString();
    m_StoreSCP->start(m_PathToStoreSCP,m_ArgumentList);
}

void QmitkStoreSCPLauncher::FindPathToStoreSCP()
{
    QString appPath= QCoreApplication::applicationDirPath();
    if(m_PathToStoreSCP.isEmpty())
    {
        QString fileName;
#ifdef _WIN32
        fileName = "/storescp.exe";
#else
        fileName = "/storescp";
#endif

        m_PathToStoreSCP = fileName;

        //In developement the storescp isn't copied into bin directory
        if(!QFile::exists(m_PathToStoreSCP))
        {
            m_PathToStoreSCP = static_cast<QString>(MITK_STORESCP);
        }
    }
}

void QmitkStoreSCPLauncher::OnProcessError(QProcess::ProcessError err)
{
    switch(err)
    {
    case QProcess::FailedToStart:
        m_ErrorText = QString("Failed to start storage provider: ").append(m_StoreSCP->errorString());
        break;
    case QProcess::Crashed:
        m_ErrorText = QString("Storage provider crashed: ").append(m_StoreSCP->errorString());
        break;
    case QProcess::Timedout:
        m_ErrorText = QString("Storage provider timeout: ").append(m_StoreSCP->errorString());
        break;
    case QProcess::WriteError:
        m_ErrorText = QString("Storage provider write error: ").append(m_StoreSCP->errorString());
        break;
    case QProcess::ReadError:
        m_ErrorText = QString("Storage provider read error: ").append(m_StoreSCP->errorString());
        break;
    case QProcess::UnknownError:
        m_ErrorText = QString("Storage provider unknown error: ").append(m_StoreSCP->errorString());
        break;
    default:
        m_ErrorText = QString("Storage provider unknown error: ").append(m_StoreSCP->errorString());
        break;
    }
}

void QmitkStoreSCPLauncher::OnStateChanged(QProcess::ProcessState status)
{
    switch(status)
    {
    case QProcess::NotRunning:
        m_StatusText = QString("Storage provider not running: ");
        emit SignalStatusOfStoreSCP(m_StatusText);
        break;
    case QProcess::Starting:
        m_StatusText = QString("Starting ").append(m_ArgumentList[2]).append(" on port ").append(m_ArgumentList[0]);
        emit SignalStatusOfStoreSCP(m_StatusText);
        break;
    case QProcess::Running:
        m_StatusText = QString("Running ").append(m_ArgumentList[2]).append(" on port ").append(m_ArgumentList[0]);;
        emit SignalStatusOfStoreSCP(m_StatusText);
        break;
    default:
        m_StatusText = QString("Storage provider unknown error: ");
        emit SignalStatusOfStoreSCP(m_StatusText);
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
