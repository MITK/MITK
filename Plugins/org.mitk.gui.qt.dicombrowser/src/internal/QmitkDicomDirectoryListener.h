/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomDirectoryListener_h
#define QmitkDicomDirectoryListener_h

#include<QObject>
#include<QString>
#include<QHash>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QDir>


#include<QTimer>

/**
* \brief QmitkDicomDirectoryListener class listens on a given directory for incomng files.
*/
class QmitkDicomDirectoryListener : public QObject
{
    Q_OBJECT

public:

    /**
    * \brief QmitkDicomDirectoryListener default constructor.
    */
    QmitkDicomDirectoryListener();

    /**
    * \brief QmitkDicomDirectoryListener default destructor.
    */
    ~QmitkDicomDirectoryListener() override;

    /**
    * \brief sets listened directory.
    * \note that only one directory can be set.
    */
    void SetDicomListenerDirectory(const QString&);

    /**
    * \brief get filepath to the listened directory.
    */
    QString GetDicomListenerDirectory();

    /**
    * \brief get the status whether the directorey listener is listening or not.
    */
    bool IsListening();

    /**
    * \brief set the directory listener listening. Id listening is set false the listener won't listen anymore.
    */
    void SetListening(bool listening);

    /**
    * \brief set m_DicomFolderSuffix.
    */
    void SetDicomFolderSuffix(QString suffix);

signals:

    /**
    * \brief signal starts the dicom import with given file list.
    */
    void SignalStartDicomImport(const QStringList&);

public slots:

    /**
    * \brief called when listener directory changes
    */
    void OnDirectoryChanged(const QString&);

    /**
    * \brief called when error occours during dicom store request
    */
    void OnDicomNetworkError(const QString&);

protected:

    /**
    * \brief creates directory if it's not already existing.
    */
    void CreateListenerDirectory(const QDir& directory);

    /**
    * \brief Composes the filename and initializes m_LastRetrievedFile with it.
    */
    void SetFilesToImport();

    /**
    * \brief removes files from listener directory.
    */
    void RemoveTemporaryFiles();

    QString m_DicomFolderSuffix;

    QFileSystemWatcher* m_FileSystemWatcher;

    QStringList m_FilesToImport;

    QHash<QString,QString> m_AlreadyImportedFiles;

    QDir m_DicomListenerDirectory;

    bool m_IsListening;
};

#endif // QmitkDicomListener_h
