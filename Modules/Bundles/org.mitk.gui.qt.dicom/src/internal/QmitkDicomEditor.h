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


#ifndef QmitkDicomEditor_h
#define QmitkDicomEditor_h

#include <berryISelectionListener.h>
#include <berryQtEditorPart.h>
#include <berryIPartListener.h>

#include <mitkIDataStorageReference.h>
#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>

#include "ui_QmitkDicomEditorControls.h"
#include "QmitkDicomDirectoryListener.h"
#include "QmitkStoreSCPLauncher.h"
#include "QmitkStoreSCPLauncherBuilder.h"
#include "DicomEventHandler.h"
#include "QmitkDicomDataEventPublisher.h"

#include <QTextEdit>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QThread.h>
#include <QProcess>

/*!
\brief QmitkDicomEditor

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkDicomEditor : public berry::QtEditorPart, virtual public berry::IPartListener
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    berryObjectMacro(QmitkDicomEditor);
    static const std::string EDITOR_ID;

    QmitkDicomEditor();
    QmitkDicomEditor(const QmitkDicomEditor& other)
    {
        Q_UNUSED(other)
            throw std::runtime_error("Copy constructor not implemented");
    }
    virtual ~QmitkDicomEditor();

    void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);

    void SetFocus();
    void DoSave() {}
    void DoSaveAs() {}
    bool IsDirty() const { return false; }
    bool IsSaveAsAllowed() const { return false; }

signals:


    protected slots:

        /// \brief Called when import is finished
        void OnDicomImportFinished(const QString& path);

        /// \brief Called when import is finished
        void OnDicomImportFinished(const QStringList& path);

        /// \brief Called when series in TreeView is double clicked.
        void OnSeriesModelDoubleClicked(QModelIndex index);

        /// \brief Called when Query Retrieve or Import Folder was clicked.
        void OnQueryRetrieve();

        /// \brief Called when LocalStorageButton was clicked.
        void OnLocalStorage();

        void StartDicomDirectoryListener(QString& directory);

        void OnChangePage(int);
        /// To be called when an entry of the tree list is collapsed
        //void OnTreeCollapsed(const QModelIndex& index);

        /// To be called when an entry of the tree list is expanded
        //void OnTreeExpanded(const QModelIndex& index);

        void TestHandler();

protected:

    void CreateQtPartControl(QWidget *parent);

    void SetupDefaults();

    Events::Types GetPartEventTypes() const;

    Ui::QmitkDicomEditorControls m_Controls;

    QThread* m_Thread;
    QmitkDicomDirectoryListener* m_DicomDirectoryListener;
    QmitkStoreSCPLauncher* m_StoreSCPLauncher;
    DicomEventHandler* m_Handler;
    QmitkDicomDataEventPublisher* m_Publisher;
 
};

#endif // QmitkDicomEditor_h

