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
#include "QmitkDicomDirectoryListener.h"

#include <QString>
#include <QFile>
#include <mitkLogMacros.h>
#include <QFileInfoList>
#include <QDirIterator>

QmitkDicomDirectoryListener::QmitkDicomDirectoryListener()
: m_FileSystemWatcher(new QFileSystemWatcher())
, m_IsListening(true)
{
    connect(m_FileSystemWatcher,SIGNAL(directoryChanged(const QString&)),this,SLOT(OnDirectoryChanged(const QString&)));
}

QmitkDicomDirectoryListener::~QmitkDicomDirectoryListener()
{
    disconnect(m_FileSystemWatcher,SIGNAL(directoryChanged(const QString&)),this,SLOT(OnDirectoryChanged(const QString&)));
    m_IsListening = false;
    RemoveTemporaryFiles();
    delete m_FileSystemWatcher;
}

void QmitkDicomDirectoryListener::OnDirectoryChanged(const QString&)
{
    if(m_IsListening)
    {
        QDirIterator it( m_DicomListenerDirectory.absolutePath() , QDir::Files , QDirIterator::Subdirectories);
        QString currentPath;

        m_FilesToImport.clear();
        while(it.hasNext())
        {
            it.next();
            currentPath = it.fileInfo().absoluteFilePath();
            if(!m_AlreadyImportedFiles.contains(currentPath))
            {
                m_AlreadyImportedFiles.insert( currentPath , currentPath );
                m_FilesToImport.append(currentPath);
            }
        }
        if(!m_FilesToImport.isEmpty())
        {
            emit SignalStartDicomImport(m_FilesToImport);
        }
    }
}

void QmitkDicomDirectoryListener::OnDicomNetworkError(const QString& errorMsg)
{
    m_IsListening = false;
    m_AlreadyImportedFiles.clear();
    m_IsListening = true;
}

void QmitkDicomDirectoryListener::RemoveTemporaryFiles()
{
    if(m_DicomListenerDirectory.absolutePath().contains(m_DicomFolderSuffix))
    {
        QDirIterator it( m_DicomListenerDirectory.absolutePath() , QDir::AllEntries , QDirIterator::Subdirectories);
        while(it.hasNext())
        {
            it.next();
            m_DicomListenerDirectory.remove(it.fileInfo().absoluteFilePath());
        }
        m_DicomListenerDirectory.rmdir(m_DicomListenerDirectory.absolutePath());
    }
}

void QmitkDicomDirectoryListener::SetDicomListenerDirectory(const QString& directory)
{
    QDir dir(directory);
    if(dir.exists())
    {
        m_DicomListenerDirectory=dir;
        m_FileSystemWatcher->addPath(m_DicomListenerDirectory.absolutePath());
    }
    else
    {
        dir.mkpath(directory);
        m_DicomListenerDirectory=dir;
        m_FileSystemWatcher->addPath(m_DicomListenerDirectory.absolutePath());
    }
}

QString QmitkDicomDirectoryListener::GetDicomListenerDirectory()
{
    return m_DicomListenerDirectory.absolutePath();
}

bool QmitkDicomDirectoryListener::IsListening()
{
    return m_IsListening;
}

void QmitkDicomDirectoryListener::SetListening(bool listening)
{
    m_IsListening = listening;
}

void QmitkDicomDirectoryListener::SetDicomFolderSuffix(QString suffix)
{
    m_DicomFolderSuffix = suffix;
}
