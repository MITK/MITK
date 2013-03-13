/*=========================================================================

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

#include <QObject>
#include <QTextEdit>
#include <QModelIndex>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QStringList>
#include <QThread>
#include <QProcess>
#include <QStringList>
#include <QLabel>
#include <QProgressDialog>
#include <ctkFileDialog.h>
#include <org_mitk_gui_qt_dicom_Export.h>

/**
* \brief QmitkDicomEditor is an editor providing functionality for dicom storage and import and query retrieve functionality.
*
* \sa berry::IPartListener
* \ingroup ${plugin_target}_internal
*/
class DICOM_EXPORT QmitkDicomEditor : public berry::QtEditorPart, virtual public berry::IPartListener
{
// this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    berryObjectMacro(QmitkDicomEditor)
    static const std::string EDITOR_ID;
    static const QString TEMP_DICOM_FOLDER_SUFFIX;

   /**
    * \brief QmitkDicomEditor constructor.
    */
    QmitkDicomEditor();

   /**
    * \brief QmitkDicomEditor destructor.
    */
    virtual ~QmitkDicomEditor();

   /**
    * \brief Init initialize the editor.
    */
    void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);

    void SetFocus();
    void DoSave() {}
    void DoSaveAs() {}
    bool IsDirty() const { return false; }
    bool IsSaveAsAllowed() const { return false; }

signals:

   /**
    * \brief SignalStartDicomImport is enitted when dicom directory for import was selected.
    */
    void SignalStartDicomImport(const QString&);

protected slots:

    /// \brief Called when import process changes.
    void OnImportProgress(int progress);

    /// \brief Called when import is finished.
    void OnDicomImportFinished();

    /// \brief Called when Query Retrieve or Import Folder was clicked.
    void OnQueryRetrieve();

    /// \brief Called when LocalStorageButton was clicked.
    void OnLocalStorage();

    /// \brief Called when FolderCDButton was clicked.
    void OnFolderCDImport();

    /// \brief Called when ok on import dialog is clicked was clicked.
    void OnFileSelected(QString);

    /// \brief Called when view button is clicked. Sends out an event for adding the current selected file to the mitkDataStorage.
    void OnViewButtonAddToDataManager(QHash<QString, QVariant> eventProperties);

    /// \brief Called when cd or folder or query retrieve button is clicked or SignalChangePage is emitted.
    void OnChangePage(int);

    /// \brief Called when status of dicom storage provider changes.
    void OnStoreSCPStatusChanged(const QString& status);

    /// \brief Called when dicom storage provider emits a network error.
    void OnDicomNetworkError(const QString& status);

protected:

    /// \brief StartStoreSCP starts  dicom storage provider.
    void StartStoreSCP();

    /// \brief StopStoreSCP stops dicom storage provider.
    void StopStoreSCP();

    /// \brief TestHandler initializes event handler.
    void TestHandler();

    /// \brief Sets database directory.
    void SetDatabaseDirectory(const QString& databaseDirectory);

    /// \brief CreateTemporaryDirectory creates temporary directory in which temorary dicom objects are stored.
    void CreateTemporaryDirectory();

    /// \brief StartDicomDirectoryListener starts dicom directory listener.
    void StartDicomDirectoryListener();

    /// \brief SetupProgressDialog Sets up progress dialog.
    void SetupProgressDialog(QWidget* parent);

    /// \brief SetupImportDialog Sets up import dialog.
    void SetupImportDialog();

   /**
    * \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_QmitkDicomeditorControls.h.
    *
    * \param parent is a pointer to the parent widget
    */
    void CreateQtPartControl(QWidget *parent);

    /// \brief SetPluginDirectory Sets plugin directory.
    void SetPluginDirectory();

    Events::Types GetPartEventTypes() const;

    ctkFileDialog* m_ImportDialog;
    QProgressDialog* m_ProgressDialog;
    QLabel* m_ProgressDialogLabel;
    Ui::QmitkDicomEditorControls m_Controls;
    QThread m_Thread;
    QmitkDicomDirectoryListener* m_DicomDirectoryListener;
    QmitkStoreSCPLauncherBuilder m_Builder;
    QmitkStoreSCPLauncher* m_StoreSCPLauncher;
    DicomEventHandler* m_Handler;
    QmitkDicomDataEventPublisher* m_Publisher;
    QString m_PluginDirectory;
    QString m_TempDirectory;
    QString m_DatabaseDirectory;

};

#endif // QmitkDicomEditor_h
