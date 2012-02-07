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
#ifndef QmitkDicomDirectoryListener_h
#define QmitkDicomDirectoryListener_h

#include<QObject>
#include<QString>
#include <QStringList>
#include <QFileSystemWatcher>


#include<QTimer>

class QmitkDicomDirectoryListener : public QObject
{
    Q_OBJECT

public:

    QmitkDicomDirectoryListener();
    
    virtual ~QmitkDicomDirectoryListener();

    /// @brief sets listened directory, note that only one directory can be set.
    void SetDicomListenerDirectory(const QString&);

signals:
    /// @brief signal starts the dicom import of the given file (the QStringList will only contain one file here).
    void StartImportingFile(QStringList&);

public slots:
    /// \brief called when listener directory changes
    void OnDirectoryChanged(const QString& changedFile);



protected:

    /// \brief Composes the filename and initializes m_LastRetrievedFile with it
    QStringList* SetRetrievedFile(const QString& filename);

    QTimer* m_Timer;
    QFileSystemWatcher* m_FileSystemWatcher;
    QStringList* m_CurrentRetrievedFile;
    QStringList* m_LastRetrievedFile;
};

#endif // QmitkDicomListener_h