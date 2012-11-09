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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPartListener.h>
#include <mitkGlobalInteraction.h>
#include <mitkDataStorageEditorInput.h>
#include "berryFileEditorInput.h"

// Qmitk
#include "QmitkDicomEditor.h"
#include "mitkPluginActivator.h"
#include <mitkDicomSeriesReader.h>

//#include "mitkProgressBar.h"

// Qt
#include <QCheckBox>
#include <QMessageBox>
#include <QWidget>

#include <QtSql>
#include <QSqlDatabase>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

//CTK
#include <ctkDICOMModel.h>
#include <ctkDICOMAppWidget.h>
#include <ctkDICOMQueryWidget.h>
#include <ctkFileDialog.h>
#include <ctkDICOMQueryRetrieveWidget.h>


const std::string QmitkDicomEditor::EDITOR_ID = "org.mitk.editors.dicomeditor";
const QString QmitkDicomEditor::TEMP_DICOM_FOLDER_SUFFIX="TmpDicomFolder";


QmitkDicomEditor::QmitkDicomEditor()
: m_Thread(new QThread())
, m_DicomDirectoryListener(new QmitkDicomDirectoryListener())
, m_StoreSCPLauncher(new QmitkStoreSCPLauncher(&m_Builder))
, m_Publisher(new QmitkDicomDataEventPublisher())
{
}

QmitkDicomEditor::~QmitkDicomEditor()
{
    m_Thread.quit();
    m_Thread.wait(1000);
    delete m_DicomDirectoryListener;
    delete m_StoreSCPLauncher;
    delete m_Handler;
    delete m_Publisher;
    delete m_ImportDialog;
}

void QmitkDicomEditor::CreateQtPartControl(QWidget *parent )
{
    m_Controls.setupUi( parent );
    m_Controls.LocalStorageButton->setIcon(QIcon(":/org.mitk.gui.qt.dicom/drive-harddisk_32.png"));
    m_Controls.FolderButton->setIcon(QIcon(":/org.mitk.gui.qt.dicom/folder_32.png"));
    m_Controls.CDButton->setIcon(QIcon(":/org.mitk.gui.qt.dicom/media-optical_32.png"));
    m_Controls.QueryRetrieveButton->setIcon(QIcon(":/org.mitk.gui.qt.dicom/network-workgroup_32.png"));
    m_Controls.StoreSCPStatusLabel->setTextFormat(Qt::RichText);
    m_Controls.StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicom/network-offline_16.png'>");


    TestHandler();

    SetPluginDirectory();
    SetDatabaseDirectory("DatabaseDirectory");
    CreateTemporaryDirectory();
    StartDicomDirectoryListener();

    SetupImportDialog();
    SetupProgressDialog(parent);

    m_Controls.m_ctkDICOMQueryRetrieveWidget->useProgressDialog(false);

    connect(m_Controls.externalDataWidget,SIGNAL(SignalStartDicomImport(const QStringList&)),m_Controls.internalDataWidget,SLOT(OnStartDicomImport(const QStringList&)));
    connect(m_Controls.externalDataWidget,SIGNAL(SignalDicomToDataManager(QHash<QString,QVariant>)),this,SLOT(OnViewButtonAddToDataManager(QHash<QString,QVariant>)));
    connect(m_Controls.externalDataWidget,SIGNAL(SignalChangePage(int)), this, SLOT(OnChangePage(int)));

    connect(m_Controls.internalDataWidget,SIGNAL(SignalFinishedImport()),this,SLOT(OnDicomImportFinished()));
    connect(m_Controls.internalDataWidget,SIGNAL(SignalDicomToDataManager(QHash<QString,QVariant>)),this,SLOT(OnViewButtonAddToDataManager(QHash<QString,QVariant>)));

    connect(m_Controls.CDButton, SIGNAL(clicked()), this, SLOT(OnFolderCDImport()));
    connect(m_Controls.FolderButton, SIGNAL(clicked()), this, SLOT(OnFolderCDImport()));
    connect(m_Controls.QueryRetrieveButton, SIGNAL(clicked()), this, SLOT(OnQueryRetrieve()));
    connect(m_Controls.LocalStorageButton, SIGNAL(clicked()), this, SLOT(OnLocalStorage()));
}

void QmitkDicomEditor::SetupProgressDialog(QWidget* parent)
{
    m_ProgressDialog = new QProgressDialog("DICOM Import", "Cancel", 0, 100, parent,Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    m_ProgressDialogLabel = new QLabel(tr("Initialization..."));
    m_ProgressDialog->setLabel(m_ProgressDialogLabel);
#ifdef Q_WS_MAC
    // BUG: avoid deadlock of dialogs on mac
    m_ProgressDialog->setWindowModality(Qt::NonModal);
#else
    m_ProgressDialog->setWindowModality(Qt::ApplicationModal);
#endif

    connect(m_ProgressDialog, SIGNAL(canceled()), m_Controls.internalDataWidget, SIGNAL(SignalCancelImport()));
    connect(m_Controls.internalDataWidget, SIGNAL(SignalProcessingFile(QString)),m_ProgressDialogLabel, SLOT(setText(QString)));
    connect(m_Controls.internalDataWidget, SIGNAL(SignalProgress(int)),m_ProgressDialog, SLOT(setValue(int)));
    connect(m_Controls.internalDataWidget, SIGNAL(SignalProgress(int)),this, SLOT(OnImportProgress(int)));

    connect(m_ProgressDialog, SIGNAL(canceled()), m_Controls.externalDataWidget, SIGNAL(SignalCancelImport()));
    connect(m_Controls.externalDataWidget, SIGNAL(SignalProcessingFile(QString)),m_ProgressDialogLabel, SLOT(setText(QString)));
    connect(m_Controls.externalDataWidget, SIGNAL(SignalProgress(int)),m_ProgressDialog, SLOT(setValue(int)));
    connect(m_Controls.externalDataWidget, SIGNAL(SignalProgress(int)),this, SLOT(OnImportProgress(int)));
}

void QmitkDicomEditor::SetupImportDialog()
{
        //Initialize import widget
        m_ImportDialog = new ctkFileDialog();
        QCheckBox* importCheckbox = new QCheckBox("Copy on import", m_ImportDialog);
        m_ImportDialog->setBottomWidget(importCheckbox);
        m_ImportDialog->setFileMode(QFileDialog::Directory);
        m_ImportDialog->setLabelText(QFileDialog::Accept,"Import");
        m_ImportDialog->setWindowTitle("Import DICOM files from directory ...");
        m_ImportDialog->setWindowModality(Qt::ApplicationModal);
        connect(m_ImportDialog, SIGNAL(fileSelected(QString)),this,SLOT(OnFileSelected(QString)));
}

void QmitkDicomEditor::OnImportProgress(int progress)
{
  Q_UNUSED(progress);
  QApplication::processEvents();
}

void QmitkDicomEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
    this->SetSite(site);
    this->SetInput(input);
}

void QmitkDicomEditor::SetFocus()
{
}

berry::IPartListener::Events::Types QmitkDicomEditor::GetPartEventTypes() const
{
    return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkDicomEditor::OnQueryRetrieve()
{
    OnChangePage(2);
    QString storagePort = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
    QString storageAET = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
     if(!((m_Builder.GetAETitle()->compare(storageAET,Qt::CaseSensitive)==0)&&
         (m_Builder.GetPort()->compare(storagePort,Qt::CaseSensitive)==0)))
     {
         StopStoreSCP();
         StartStoreSCP();
     }
}

void QmitkDicomEditor::OnFileSelected(QString directory)
{
    if (QDir(directory).exists())
    {
        QCheckBox* copyOnImport = qobject_cast<QCheckBox*>(m_ImportDialog->bottomWidget());

        if (copyOnImport->isChecked())
        {
            connect(this,SIGNAL(SignalStartDicomImport(const QString&)),m_Controls.internalDataWidget,SLOT(OnStartDicomImport(const QString&)));
            disconnect(this,SIGNAL(SignalStartDicomImport(const QString&)),m_Controls.externalDataWidget,SLOT(OnStartDicomImport(const QString&)));
            OnChangePage(0);
        }
        else
        {
            disconnect(this,SIGNAL(SignalStartDicomImport(const QString&)),m_Controls.internalDataWidget,SLOT(OnStartDicomImport(const QString&)));
            connect(this,SIGNAL(SignalStartDicomImport(const QString&)),m_Controls.externalDataWidget,SLOT(OnStartDicomImport(const QString&)));
            OnChangePage(1);
        }

        m_ProgressDialog->setMinimumDuration(0);
        m_ProgressDialog->setValue(0);
        m_ProgressDialog->show();
        emit SignalStartDicomImport(directory);
    }
}

void QmitkDicomEditor::OnFolderCDImport()
{
    m_ImportDialog->show();
    m_ImportDialog->raise();
}

void QmitkDicomEditor::OnLocalStorage()
{
    OnChangePage(0);
}

void QmitkDicomEditor::OnChangePage(int page)
{
    try{
        m_Controls.stackedWidget->setCurrentIndex(page);
    }catch(std::exception e){
        MITK_ERROR <<"error: "<< e.what();
        return;
    }
}

void QmitkDicomEditor::OnDicomImportFinished()
{
}

void QmitkDicomEditor::StartDicomDirectoryListener()
{
    if(!m_Thread.isRunning())
    {
        m_DicomDirectoryListener->SetDicomListenerDirectory(m_TempDirectory);
        m_DicomDirectoryListener->SetDicomFolderSuffix(TEMP_DICOM_FOLDER_SUFFIX);
        connect(m_DicomDirectoryListener,SIGNAL(SignalStartDicomImport(const QStringList&)),m_Controls.internalDataWidget,SLOT(OnStartDicomImport(const QStringList&)),Qt::DirectConnection);
        //connect(m_Controls.internalDataWidget,SIGNAL(SignalFinishedImport()),m_DicomDirectoryListener,SLOT(OnImportFinished()),Qt::DirectConnection);
        m_DicomDirectoryListener->moveToThread(&m_Thread);
        m_Thread.start();
    }
}


void QmitkDicomEditor::TestHandler()
{
    m_Handler = new DicomEventHandler();
    m_Handler->SubscribeSlots();
}

void QmitkDicomEditor::OnViewButtonAddToDataManager(QHash<QString, QVariant> eventProperties)
{
    ctkDictionary properties;
    properties["PatientName"] = eventProperties["PatientName"];
    properties["StudyUID"] = eventProperties["StudyUID"];
    properties["StudyName"] = eventProperties["StudyName"];
    properties["SeriesUID"] = eventProperties["SeriesUID"];
    properties["SeriesName"] = eventProperties["SeriesName"];
    properties["FilesForSeries"] = eventProperties["FilesForSeries"];

    m_Publisher->PublishSignals(mitk::PluginActivator::getContext());
    m_Publisher->AddSeriesToDataManagerEvent(properties);
}


void QmitkDicomEditor::StartStoreSCP()
{
    QString storagePort = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
    QString storageAET = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
    m_Builder.AddPort(storagePort)->AddAETitle(storageAET)->AddTransferSyntax()->AddOtherNetworkOptions()->AddMode()->AddOutputDirectory(m_TempDirectory);
    m_StoreSCPLauncher = new QmitkStoreSCPLauncher(&m_Builder);
    connect(m_StoreSCPLauncher, SIGNAL(SignalStatusOfStoreSCP(const QString&)), this, SLOT(OnStoreSCPStatusChanged(const QString&)));
    connect(m_StoreSCPLauncher ,SIGNAL(SignalStartImport(const QStringList&)),m_Controls.internalDataWidget,SLOT(OnStartDicomImport(const QStringList&)));
    connect(m_StoreSCPLauncher ,SIGNAL(SignalStoreSCPError(const QString&)),m_Controls.internalDataWidget,SLOT(SignalCancelImport()));
    connect(m_StoreSCPLauncher ,SIGNAL(SignalStoreSCPError(const QString&)),m_DicomDirectoryListener,SLOT(OnDicomNetworkError(const QString&)),Qt::DirectConnection);
    connect(m_StoreSCPLauncher ,SIGNAL(SignalStoreSCPError(const QString&)),this,SLOT(OnDicomNetworkError(const QString&)),Qt::DirectConnection);
    m_StoreSCPLauncher->StartStoreSCP();

}

void QmitkDicomEditor::OnStoreSCPStatusChanged(const QString& status)
{
    m_Controls.StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicom/network-idle_16.png'> "+status);
}

void QmitkDicomEditor::OnDicomNetworkError(const QString& status)
{
    m_Controls.StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicom/network-error_16.png'> "+status);
}

void QmitkDicomEditor::StopStoreSCP()
{
    delete m_StoreSCPLauncher;
}

void QmitkDicomEditor::SetPluginDirectory()
{
     m_PluginDirectory = mitk::PluginActivator::getContext()->getDataFile("").absolutePath();
     m_PluginDirectory.append("/");
}

void QmitkDicomEditor::SetDatabaseDirectory(const QString& databaseDirectory)
{
    m_DatabaseDirectory.clear();
    m_DatabaseDirectory.append(m_PluginDirectory);
    m_DatabaseDirectory.append(databaseDirectory);
    m_Controls.internalDataWidget->SetDatabaseDirectory(m_DatabaseDirectory);
}

void QmitkDicomEditor::CreateTemporaryDirectory()
{
    QDir tmp;
    QString tmpPath = QDir::tempPath();
    m_TempDirectory.clear();
    m_TempDirectory.append(tmpPath);
    m_TempDirectory.append(QString("/"));
    m_TempDirectory.append(TEMP_DICOM_FOLDER_SUFFIX);
    m_TempDirectory.append(QString("."));
    m_TempDirectory.append(QTime::currentTime().toString("hhmmsszzz"));
    m_TempDirectory.append(QString::number(QCoreApplication::applicationPid()));
    tmp.mkdir(QDir::toNativeSeparators( m_TempDirectory ));

}
