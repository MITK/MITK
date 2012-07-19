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

#ifndef QmitkDicomDirectoryListener_h
#define QmitkDicomDirectoryListener_h

#include<QObject>
#include<QString>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QDir>
#include <QMutex>


#include<QTimer>

class QmitkDicomDirectoryListener : public QObject
{
    Q_OBJECT

public:

    QmitkDicomDirectoryListener();
    
    virtual ~QmitkDicomDirectoryListener();

    /// @brief sets listened directory, note that only one directory can be set.
    void SetDicomListenerDirectory(const QString&);

    /// @brief get filepath to the listened directory.
    const QString& GetDicomListenerDirectory();

signals:
    /// @brief signal starts the dicom import of the given file (the QStringList will only contain one file here).
    void SignalAddDicomData(const QStringList&);

public slots:
    /// \brief called when listener directory changes
    void OnDirectoryChanged(const QString&);

    /// \brief called when import is finished
    void OnDicomImportFinished(const QStringList&);



protected:

    /// \brief creates directory if it's not already existing.  
    void CreateListenerDirectory(const QDir& directory);

    /// \brief checks wheter the given directory is the only directory that is listened.
    bool isOnlyListenedDirectory(const QString& directory);

    /// \brief Composes the filename and initializes m_LastRetrievedFile with it
    void SetFilesToImport();

    /// \brief removes files from 
    void RemoveFilesFromDirectoryAndImportingFilesList(const QStringList& files);

    QFileSystemWatcher* m_FileSystemWatcher;
    QStringList* m_FilesToImport;
    QStringList* m_ImportingFiles;
    QString m_DicomListenerDirectory;
    QMutex m_Mutex;
};

#endif // QmitkDicomListener_h