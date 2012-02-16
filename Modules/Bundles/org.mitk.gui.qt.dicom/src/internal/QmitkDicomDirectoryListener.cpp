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

#include "QmitkDicomDirectoryListener.h"

#include <QString>
#include <QFile>

#include <QFileInfoList>

QmitkDicomDirectoryListener::QmitkDicomDirectoryListener()
: m_LastRetrievedFile(new QStringList())
, m_CurrentRetrievedFile(new QStringList())
, m_FileSystemWatcher(new QFileSystemWatcher())
, m_Timer(new QTimer(this))
, m_DicomListenerDirectory(QString())
{
    m_Timer->setSingleShot(true);
    m_Timer->blockSignals(true);
    connect(m_FileSystemWatcher,SIGNAL(directoryChanged(const QString&)),this,SLOT(OnDirectoryChanged(const QString&))); 

}

QmitkDicomDirectoryListener::~QmitkDicomDirectoryListener()
{
    delete m_FileSystemWatcher;
    delete m_CurrentRetrievedFile;
    delete m_LastRetrievedFile;
    delete m_Timer;
}


void QmitkDicomDirectoryListener::OnDirectoryChanged(const QString& changedFile)
{   

    // set m_CurrentRetrievedFile stays empty if the folder doesn't contain any new files e.g. if a file was deleted  
    m_CurrentRetrievedFile = SetRetrievedFile(changedFile);

    //Situation: directory is empty and there is a new incoming file
    if(!m_Timer->isActive() && !m_CurrentRetrievedFile->isEmpty())
    {
        m_Timer->start(30000);      
        emit StartImportingFile(*m_CurrentRetrievedFile);        
        m_LastRetrievedFile = m_CurrentRetrievedFile;
        m_CurrentRetrievedFile->clear();
    }
    //Situation: A new file comes in and the timer is still running -> indicates the last download is finished
    if(m_Timer->isActive()&& !m_CurrentRetrievedFile->isEmpty())
    {     
        m_Timer->stop(); 
        emit StartImportingFile(*m_LastRetrievedFile);        
        m_Timer->start(30000);      
        emit StartImportingFile(*m_CurrentRetrievedFile);        
        m_LastRetrievedFile = m_CurrentRetrievedFile;
        m_CurrentRetrievedFile->clear();
    }

}

QStringList* QmitkDicomDirectoryListener::SetRetrievedFile(const QString& filename)
{
    QDir listenerDirectory(filename);
    //if the listenordirectory is empty you have deleted the last file
    if(!listenerDirectory.entryList().isEmpty())
    {
        QFileInfoList directoryMembers;
        directoryMembers = listenerDirectory.entryInfoList();
        QFileInfoList::iterator member;
        member=directoryMembers.begin();
        QStringList temp;

        //filter files, no directories wanted
        while(member!=directoryMembers.end())
        {
            if((*member).isFile()){
                temp.append((*member).absoluteFilePath());
            }
            ++member;
        }

        //check if there is an existing new file
        if(!temp.isEmpty())
        {
            return new QStringList(temp);
        }
    }
    return new QStringList();
}

void QmitkDicomDirectoryListener::SetDicomListenerDirectory(const QString& directory)
{
    if(isOnlyListenedDirectory(directory))
    {
        QDir listenerDirectory = QDir(directory);
        CreateListenerDirectory(listenerDirectory);
        
        m_DicomListenerDirectory=listenerDirectory.absolutePath();
        m_FileSystemWatcher->addPath(m_DicomListenerDirectory);
    }

}

const QString& QmitkDicomDirectoryListener::GetDicomListenerDirectory()
{
    return m_DicomListenerDirectory;
}

void QmitkDicomDirectoryListener::CreateListenerDirectory(const QDir& directory)
{   
    if(!directory.exists())
    {
        directory.mkpath(directory.absolutePath());
    }
}

bool QmitkDicomDirectoryListener::isOnlyListenedDirectory(const QString& directory)
{
    bool isOnlyListenedDirectory = false;
    if(m_FileSystemWatcher->directories().count()==0||m_FileSystemWatcher->directories().count()==1)
    {
        if(!m_FileSystemWatcher->directories().contains(directory))
        {
            isOnlyListenedDirectory = true;
        }
    }
    return isOnlyListenedDirectory;
}