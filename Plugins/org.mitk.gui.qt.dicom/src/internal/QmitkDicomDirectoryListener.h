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
#include<QHash>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QDir>


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
    QString GetDicomListenerDirectory();

    /// @brief get the status whether the directorey listener is listening or not.
    bool IsListening();

    /// @brief set the directory listener listening.
    void SetListening(bool listening);

    /// @brief set m_DicomFolderSuffix.
    void SetDicomFolderSuffix(QString suffix);

signals:
    /// @brief signal starts the dicom import of the given file list.
    void SignalStartDicomImport(const QStringList&);

public slots:
    /// \brief called when listener directory changes
    void OnDirectoryChanged(const QString&);

    /// \brief called when error occours during dicom store request
    void OnDicomNetworkError(const QString&);

protected:

    /// \brief creates directory if it's not already existing.
    void CreateListenerDirectory(const QDir& directory);

    /// \brief Composes the filename and initializes m_LastRetrievedFile with it.
    void SetFilesToImport();

    /// \brief removes files from listener directory.
    void RemoveTemporaryFiles();

    QString m_DicomFolderSuffix;

    QFileSystemWatcher* m_FileSystemWatcher;

    QStringList m_FilesToImport;

    QHash<QString,QString> m_AlreadyImportedFiles;

    QDir m_DicomListenerDirectory;

    bool m_IsListening;
};

#endif // QmitkDicomListener_h