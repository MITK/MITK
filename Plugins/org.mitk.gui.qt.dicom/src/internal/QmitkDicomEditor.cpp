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

// Qmitk
#include "QmitkDicomEditor.h"
#include "mitkPluginActivator.h"


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
}

void QmitkDicomEditor::CreateQtPartControl(QWidget *parent )
{
    m_Controls.setupUi( parent );
    m_Controls.StoreSCPStatusLabel->setTextFormat(Qt::RichText);
    m_Controls.StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicom/network-offline_16.png'>");


    TestHandler();

    SetPluginDirectory();
    SetDatabaseDirectory("DatabaseDirectory");
    CreateTemporaryDirectory();
    StartDicomDirectoryListener();

    m_Controls.m_ctkDICOMQueryRetrieveWidget->useProgressDialog(false);

    connect(m_Controls.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)));

    connect(m_Controls.externalDataWidget,SIGNAL(SignalStartDicomImport(const QStringList&)),
            m_Controls.internalDataWidget,SLOT(OnStartDicomImport(const QStringList&)));

    connect(m_Controls.externalDataWidget,SIGNAL(SignalDicomToDataManager(const QHash<QString,QVariant>&)),
            this,SLOT(OnViewButtonAddToDataManager(const QHash<QString,QVariant>&)));

    connect(m_Controls.internalDataWidget,SIGNAL(SignalFinishedImport()),this, SLOT(OnDicomImportFinished()));
    connect(m_Controls.internalDataWidget,SIGNAL(SignalDicomToDataManager(const QHash<QString,QVariant>&)),
            this,SLOT(OnViewButtonAddToDataManager(const QHash<QString,QVariant>&)));
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

void QmitkDicomEditor::OnTabChanged(int page)
{
  if (page == 2)//Query/Retrieve is selected
  {
    QString storagePort = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
    QString storageAET = m_Controls.m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
     if(!((m_Builder.GetAETitle()->compare(storageAET,Qt::CaseSensitive)==0)&&
         (m_Builder.GetPort()->compare(storagePort,Qt::CaseSensitive)==0)))
     {
         StopStoreSCP();
         StartStoreSCP();
     }
  }
}

void QmitkDicomEditor::OnDicomImportFinished()
{
  m_Controls.tabWidget->setCurrentIndex(0);
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
//    properties["PatientName"] = eventProperties["PatientName"];
//    properties["StudyUID"] = eventProperties["StudyUID"];
//    properties["StudyName"] = eventProperties["StudyName"];
//    properties["SeriesUID"] = eventProperties["SeriesUID"];
//    properties["SeriesName"] = eventProperties["SeriesName"];
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
