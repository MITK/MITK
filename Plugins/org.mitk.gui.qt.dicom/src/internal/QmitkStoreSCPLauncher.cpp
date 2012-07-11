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
#include <mitkLogMacros.h>
#include <fstream>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDir>

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
    SetPathToStoreSCP();
    m_StoreSCP->start(m_PathToStoreSCP,m_ArgumentList);
}

void QmitkStoreSCPLauncher::SetPathToStoreSCP()
{
    QDir dir("qrc:///org.mitk.gui.qt.dicom/config.txt");
    MITK_INFO << dir.absolutePath().toStdString();
    QFile file((dir.absolutePath());
    m_PathToStoreSCP.clear();

    if(file.open(QIODevice::ReadWrite))
    {
        QTextStream stream ( &file );
        QString line;

        while( !stream.atEnd() )
        {
            line = stream.readLine();
            MITK_INFO << line.toStdString();
            m_PathToStoreSCP.append(line);
            file.close();
        }
    }else{
        MITK_INFO << "Unable to open file"; 
    }
}

void QmitkStoreSCPLauncher::OnProcessError(QProcess::ProcessError err)
{
    switch(err)
    {
    case QProcess::FailedToStart:
        MITK_INFO << QString("Failed to start storage provider: ").append(m_StoreSCP->errorString()).toStdString();
        break;
    case QProcess::Crashed:
        MITK_INFO << QString("Storage provider crashed: ").append(m_StoreSCP->errorString()).toStdString();
        break;
    case QProcess::Timedout:
        MITK_INFO << QString("Storage provider timeout: ").append(m_StoreSCP->errorString()).toStdString();
        break;
    case QProcess::WriteError:
        MITK_INFO << QString("Storage provider write error: ").append(m_StoreSCP->errorString()).toStdString();
        break;
    case QProcess::ReadError:
        MITK_INFO << QString("Storage provider read error: ").append(m_StoreSCP->errorString()).toStdString();
        break;
    case QProcess::UnknownError:
        MITK_INFO << QString("Storage provider unknown error: ").append(m_StoreSCP->errorString()).toStdString();
        break;
    default:
        MITK_INFO << QString("Storage provider unknown error: ").append(m_StoreSCP->errorString()).toStdString();
        break;
    }
}

void QmitkStoreSCPLauncher::OnStateChanged(QProcess::ProcessState status)
{
    switch(status)
    {
    case QProcess::NotRunning:
        MITK_INFO << QString("Storage provider not running: ").append(m_StoreSCP->errorString()).toStdString();
        break;
    case QProcess::Starting:
        MITK_INFO << QString("Starting storage provider").toStdString();
        break;
    case QProcess::Running:
        MITK_INFO << QString("Running storage provider").toStdString();
        break;
    default:
        MITK_INFO << QString("Storage provider unknown error: ").append(m_StoreSCP->errorString()).toStdString();
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
