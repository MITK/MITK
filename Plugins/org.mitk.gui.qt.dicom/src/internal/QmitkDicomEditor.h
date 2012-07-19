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

#ifndef QmitkDicomEditor_h
#define QmitkDicomEditor_h

#include <berryISelectionListener.h>
#include <berryQtEditorPart.h>
#include <berryIPartListener.h>

#include "ui_QmitkDicomEditorControls.h"
#include "QmitkDicomDirectoryListener.h"
#include "QmitkStoreSCPLauncher.h"
#include "QmitkStoreSCPLauncherBuilder.h"
#include "DicomEventHandler.h"
#include "QmitkDicomDataEventPublisher.h"

#include <QTextEdit>
#include <QModelIndex>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QStringList>
#include <QThread>
#include <QProcess>
#include <QStringList>
#include <org_mitk_gui_qt_dicom_Export.h>
/*!
\brief QmitkDicomEditor

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class DICOM_EXPORT QmitkDicomEditor : public berry::QtEditorPart, virtual public berry::IPartListener
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    berryObjectMacro(QmitkDicomEditor)
    static const std::string EDITOR_ID;

    QmitkDicomEditor();

    virtual ~QmitkDicomEditor();

    void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);

    void SetFocus();
    void DoSave() {}
    void DoSaveAs() {}
    bool IsDirty() const { return false; }
    bool IsSaveAsAllowed() const { return false; }
    
signals:


    protected slots:

        /// \brief Called when StoreSCP shold start
        void StartStoreSCP();

        /// \brief Called when StoreSCP should stop
        void StopStoreSCP();

        /// \brief Called when import is finished
        void OnDicomImportFinished(const QString& path);

        /// \brief Called when import is finished
        void OnDicomImportFinished(const QStringList& path);

        /// \brief Called when Query Retrieve or Import Folder was clicked.
        void OnQueryRetrieve();

        /// \brief Called when LocalStorageButton was clicked.
        void OnLocalStorage();

        /// \brief Called when FolderCDButton was clicked.
        void OnFolderCDImport();

        /// \brief Called when view button is clicked. Sends out an event for adding the current selected file to the mitkDataStorage.
        void OnViewButtonAddToDataManager(const QStringList& eventProperties);

        void StartDicomDirectoryListener();

        void OnChangePage(int);

        void TestHandler();

        void SetDatabaseDirectory(const QString& databaseDirectory);
    
        void SetListenerDirectory(const QString& listenerDirectory);

protected:

    void CreateQtPartControl(QWidget *parent);

    void SetPluginDirectory();

    Events::Types GetPartEventTypes() const;

    Ui::QmitkDicomEditorControls m_Controls;

    QThread* m_Thread;
    QmitkDicomDirectoryListener* m_DicomDirectoryListener;
    QmitkStoreSCPLauncherBuilder builder;
    QmitkStoreSCPLauncher* m_StoreSCPLauncher;
    DicomEventHandler* m_Handler;
    QmitkDicomDataEventPublisher* m_Publisher;
    QString m_PluginDirectory;
    QString m_ListenerDirectory;
    QString m_DatabaseDirectory;

};

#endif // QmitkDicomEditor_h
