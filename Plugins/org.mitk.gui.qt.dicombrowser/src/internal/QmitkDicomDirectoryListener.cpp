/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomDirectoryListener.h"

#include <mitkLog.h>

#include <QDirIterator>
#include <QFileSystemWatcher>

using Self = QmitkDicomDirectoryListener;

QmitkDicomDirectoryListener::QmitkDicomDirectoryListener()
  : m_FileSystemWatcher(new QFileSystemWatcher),
    m_IsListening(true)
{
  connect(m_FileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &Self::OnDirectoryChanged);
}

QmitkDicomDirectoryListener::~QmitkDicomDirectoryListener()
{
  disconnect(m_FileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &Self::OnDirectoryChanged);
  m_IsListening = false;
  this->RemoveTemporaryFiles();
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

void QmitkDicomDirectoryListener::OnDicomNetworkError(const QString& /*errorMsg*/)
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
  }
}

void QmitkDicomDirectoryListener::SetDicomListenerDirectory(const QString& directory)
{
  QDir dir(directory);

  if (!dir.exists())
    dir.mkpath(directory);

  m_DicomListenerDirectory=dir;
  m_FileSystemWatcher->addPath(m_DicomListenerDirectory.absolutePath());
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
