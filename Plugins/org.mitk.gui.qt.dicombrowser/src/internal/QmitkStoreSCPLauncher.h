/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStoreSCPLauncher_h
#define QmitkStoreSCPLauncher_h

#include <QProcess>
#include <QString>
#include "QmitkStoreSCPLauncherBuilder.h"

/**
* \brief QmitkStoreSCPLauncher launches the dcmtk storage provider commandline tool in another process and provides basic interoperability with it.
*/
class QmitkStoreSCPLauncher : public QObject
{
    Q_OBJECT

public:

    /**
    * \brief QmitkStoreSCPLauncher constructor.
    */
    QmitkStoreSCPLauncher(QmitkStoreSCPLauncherBuilder* builder);

    /**
    * \brief QmitkStoreSCPLauncher constructor.
    */
    ~QmitkStoreSCPLauncher() override;

public slots:

    /**
    * \brief Starts a storage provider in a new process.
    */
    void StartStoreSCP();

    /**
    * \brief Called when storage provider process emits error.
    */
    void OnProcessError(QProcess::ProcessError error);

    /**
    * \brief Called when storage provider process changes its state.
    */
    void OnStateChanged(QProcess::ProcessState status);

    /**
    * \brief Called when storage provider process provides new information on its standard output.
    */
    void OnReadyProcessOutput();

signals:

    /**
    * \brief signal is emitted if status of storage provider process has changend.
    * \param QString containing m_StatusText.
    */
    void SignalStatusOfStoreSCP(const QString&);

    /**
    * \brief signal is emitted an error occours while storage provider process is running.
    * \param QString containing m_ErrorText.
    */
    void SignalStoreSCPError(const QString& errorText = "");

    /**
    * \brief signal is emitted when storage provider process starts storing dicom files.
    * \param QStringList& of processed dicom files.
    */
    void SignalStartImport(const QStringList&);

    /**
    * \brief signal is emitted if import of storage provider process has finished.
    * \note currently not used.
    */
    void SignalFinishedImport();

private:

    /**
    * \brief DicomEventHandler constructor.
    */
    void FindPathToStoreSCP();

    /**
    * \brief DicomEventHandler constructor.
    * \param QmitkStoreSCPLauncherBuilder* to builder object.
    */
    void SetArgumentList(QmitkStoreSCPLauncherBuilder* builder);

    /**
    * \brief Converts the m_ArgumentList into a QString containing all arguments from list.
    */
    QString ArgumentListToQString();

    QString m_PathToStoreSCP;
    QString m_ErrorText;
    QString m_StatusText;

    QProcess* m_StoreSCP;
    QStringList m_ArgumentList;
    QStringList m_ImportFilesList;
};
#endif
