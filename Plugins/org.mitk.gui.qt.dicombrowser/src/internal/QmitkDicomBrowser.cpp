/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomBrowser.h"
#include <ui_QmitkDicomBrowser.h>

#include "QmitkDicomEventHandler.h"
#include "mitkPluginActivator.h"
#include "QmitkDicomDataEventPublisher.h"
#include "QmitkDicomDirectoryListener.h"
#include "QmitkStoreSCPLauncher.h"

using Self = QmitkDicomBrowser;

const std::string QmitkDicomBrowser::EDITOR_ID = "org.mitk.editors.dicombrowser";
const QString QmitkDicomBrowser::TEMP_DICOM_FOLDER_SUFFIX = "TmpDicomFolder";

QmitkDicomBrowser::QmitkDicomBrowser()
  : m_Ui(new Ui::QmitkDicomBrowser),
    m_ImportDialog(nullptr),
    m_DicomDirectoryListener(std::make_unique<QmitkDicomDirectoryListener>()),
    m_StoreSCPLauncher(std::make_unique<QmitkStoreSCPLauncher>(&m_Builder)),
    m_Publisher(std::make_unique<QmitkDicomDataEventPublisher>())
{
}

QmitkDicomBrowser::~QmitkDicomBrowser()
{
}

void QmitkDicomBrowser::CreateQtPartControl(QWidget *parent )
{
  m_Ui->setupUi( parent );
  m_Ui->statusLabel->setTextFormat(Qt::RichText);
  m_Ui->statusLabel->setText("<img src=':/org.mitk.gui.qt.dicombrowser/network-offline_16.png'>");

  this->TestHandler();

  m_PluginDirectory = mitk::PluginActivator::GetContext()->getDataFile("").absolutePath();
  m_DatabaseDirectory = m_PluginDirectory.append("/database");
  m_Ui->internalDataWidget->SetDatabaseDirectory(m_DatabaseDirectory);

  this->CreateTemporaryDirectory();
  this->StartDicomDirectoryListener();

  m_Ui->queryRetrieveWidget->useProgressDialog(true);

  connect(m_Ui->tabWidget, &QTabWidget::currentChanged,
          this, &Self::OnTabChanged);

  connect(m_Ui->externalDataWidget, &QmitkDicomExternalDataWidget::SignalStartDicomImport,
          m_Ui->internalDataWidget, qOverload<const QStringList&>(&QmitkDicomLocalStorageWidget::OnStartDicomImport));

  connect(m_Ui->externalDataWidget, &QmitkDicomExternalDataWidget::SignalDicomToDataManager,
          this, &Self::OnViewButtonAddToDataManager);

  connect(m_Ui->internalDataWidget, &QmitkDicomLocalStorageWidget::SignalFinishedImport,
          this, &Self::OnDicomImportFinished);

  connect(m_Ui->internalDataWidget, &QmitkDicomLocalStorageWidget::SignalDicomToDataManager,
          this, &Self::OnViewButtonAddToDataManager);
}


void QmitkDicomBrowser::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  this->SetSite(site);
  this->SetInput(input);
}

void QmitkDicomBrowser::SetFocus()
{
}

berry::IPartListener::Events::Types QmitkDicomBrowser::GetPartEventTypes() const
{
  return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkDicomBrowser::OnTabChanged(int page)
{
  if (page == 2) //Query/Retrieve is selected
  {
    auto storagePort = m_Ui->queryRetrieveWidget->getServerParameters()["StoragePort"].toString();
    auto storageAET = m_Ui->queryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();

    if(!((m_Builder.GetAETitle()->compare(storageAET) == 0) &&
         (m_Builder.GetPort()->compare(storagePort) == 0)))
    {
      this->StopStoreSCP();
      this->StartStoreSCP();
    }
  }
}

void QmitkDicomBrowser::OnDicomImportFinished()
{
  m_Ui->tabWidget->setCurrentIndex(0);
}

void QmitkDicomBrowser::StartDicomDirectoryListener()
{
  m_DicomDirectoryListener->SetDicomListenerDirectory(m_TempDirectory);
  m_DicomDirectoryListener->SetDicomFolderSuffix(TEMP_DICOM_FOLDER_SUFFIX);

  connect(m_DicomDirectoryListener.get(), &QmitkDicomDirectoryListener::SignalStartDicomImport,
          m_Ui->internalDataWidget, qOverload<const QStringList&>(&QmitkDicomLocalStorageWidget::OnStartDicomImport),
          Qt::DirectConnection);
}

void QmitkDicomBrowser::TestHandler()
{
  m_Handler = std::make_unique<QmitkDicomEventHandler>();
  m_Handler->SubscribeSlots();
}

void QmitkDicomBrowser::OnViewButtonAddToDataManager(const QHash<QString, QVariant>& eventProperties)
{
  ctkDictionary properties;
  properties["FilesForSeries"] = eventProperties["FilesForSeries"];

  if (eventProperties.contains("Modality"))
    properties["Modality"] = eventProperties["Modality"];

  m_Publisher->PublishSignals(mitk::PluginActivator::GetContext());
  m_Publisher->AddSeriesToDataManagerEvent(properties);
}

void QmitkDicomBrowser::StartStoreSCP()
{
  auto storagePort = m_Ui->queryRetrieveWidget->getServerParameters()["StoragePort"].toString();
  auto storageAET = m_Ui->queryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
  m_Builder.AddPort(storagePort)->AddAETitle(storageAET)->AddTransferSyntax()->AddOtherNetworkOptions()->AddMode()->AddOutputDirectory(m_TempDirectory);
  m_StoreSCPLauncher = std::make_unique<QmitkStoreSCPLauncher>(&m_Builder);

  connect(m_StoreSCPLauncher.get(), &QmitkStoreSCPLauncher::SignalStatusOfStoreSCP,
          this, &Self::OnStoreSCPStatusChanged);

  connect(m_StoreSCPLauncher.get(), &QmitkStoreSCPLauncher::SignalStartImport,
          m_Ui->internalDataWidget, qOverload<const QStringList&>(&QmitkDicomLocalStorageWidget::OnStartDicomImport));

  connect(m_StoreSCPLauncher.get(), &QmitkStoreSCPLauncher::SignalStoreSCPError,
          m_DicomDirectoryListener.get(), &QmitkDicomDirectoryListener::OnDicomNetworkError,
          Qt::DirectConnection);

  connect(m_StoreSCPLauncher.get(), &QmitkStoreSCPLauncher::SignalStoreSCPError,
          this, &Self::OnDicomNetworkError,
          Qt::DirectConnection);

  m_StoreSCPLauncher->StartStoreSCP();
}

void QmitkDicomBrowser::OnStoreSCPStatusChanged(const QString& status)
{
  m_Ui->statusLabel->setText("<img src=':/org.mitk.gui.qt.dicombrowser/network-idle_16.png'> " + status);
}

void QmitkDicomBrowser::OnDicomNetworkError(const QString& status)
{
  m_Ui->statusLabel->setText("<img src=':/org.mitk.gui.qt.dicombrowser/network-error_16.png'> " + status);
}

void QmitkDicomBrowser::StopStoreSCP()
{
  m_StoreSCPLauncher.reset();
}

void QmitkDicomBrowser::CreateTemporaryDirectory()
{
  const QString tempPath = QDir::tempPath();

  m_TempDirectory.clear();
  m_TempDirectory.append(tempPath);
  m_TempDirectory.append(QString("/"));
  m_TempDirectory.append(TEMP_DICOM_FOLDER_SUFFIX);
  m_TempDirectory.append(QString("."));
  m_TempDirectory.append(QTime::currentTime().toString("hhmmsszzz"));
  m_TempDirectory.append(QString::number(QCoreApplication::applicationPid()));

  QDir tmp;
  tmp.mkdir(QDir::toNativeSeparators(m_TempDirectory));
}
