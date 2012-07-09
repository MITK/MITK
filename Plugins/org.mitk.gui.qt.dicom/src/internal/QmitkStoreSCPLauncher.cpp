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
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <fstream>
#include <iostream>

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
    std::string line;
    std::ifstream myfile("text.ini");
    if (myfile.is_open())
    {
        while ( myfile.good() )
        {
            std::getline (myfile,line);
            MITK_INFO << line << endl;
        }
        myfile.close();
    }else{
        MITK_INFO << "Unable to open file"; 
    }
    m_PathToStoreSCP.fromStdString(line);
    MITK_INFO << m_PathToStoreSCP.toStdString();
    //if(QFile::exists("config.ini")){


    //    if (!file.open(QIODevice::ReadWrite))
    //    {
    //        MITK_INFO << "cannot find file";
    //    }
    //    QTextStream stream(&file);

    //    while( !stream.atEnd() ) {
    //        m_PathToStoreSCP = stream.readLine().split("-").at(1);
    //    }
    //    file.close();
    //    MITK_INFO << m_PathToStoreSCP.toStdString();
    //    if(m_PathToStoreSCP.compare("NOTFOUND"))
    //    {
    //        MITK_INFO << "cannot find binary";
    //    }
    //}
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
