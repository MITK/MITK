/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomBrowser.h"
#include <ui_QmitkDicomBrowserControls.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>

#include "DicomEventHandler.h"
#include "mitkPluginActivator.h"
#include "QmitkDicomDataEventPublisher.h"
#include "QmitkDicomDirectoryListener.h"
#include "QmitkStoreSCPLauncher.h"

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto prefsService = mitk::CoreServices::GetPreferencesService();
    return prefsService->GetSystemPreferences()->Node("org.mitk.views.dicomreader");
  }
}

using Self = QmitkDicomBrowser;

const std::string QmitkDicomBrowser::EDITOR_ID = "org.mitk.editors.dicombrowser";
const QString QmitkDicomBrowser::TEMP_DICOM_FOLDER_SUFFIX = "TmpDicomFolder";

QmitkDicomBrowser::QmitkDicomBrowser()
  : m_Controls(new Ui::QmitkDicomBrowserControls),
    m_ImportDialog(nullptr),
    m_DicomDirectoryListener(new QmitkDicomDirectoryListener()),
    m_StoreSCPLauncher(new QmitkStoreSCPLauncher(&m_Builder)),
    m_Handler(nullptr),
    m_Publisher(new QmitkDicomDataEventPublisher())
{
  if (auto prefs = GetPreferences(); prefs != nullptr)
    prefs->OnChanged.AddListener(mitk::MessageDelegate1<Self, const mitk::IPreferences*>(this, &Self::OnPreferencesChanged));
}

QmitkDicomBrowser::~QmitkDicomBrowser()
{
  if (auto prefs = GetPreferences(); prefs != nullptr)
    prefs->OnChanged.RemoveListener(mitk::MessageDelegate1<Self, const mitk::IPreferences*>(this, &Self::OnPreferencesChanged));

  delete m_DicomDirectoryListener;
  delete m_StoreSCPLauncher;
  delete m_Handler;
  delete m_Publisher;
}

void QmitkDicomBrowser::CreateQtPartControl(QWidget *parent )
{
  m_Controls->setupUi( parent );
  m_Controls->StoreSCPStatusLabel->setTextFormat(Qt::RichText);
  m_Controls->StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicombrowser/network-offline_16.png'>");

  this->TestHandler();

  this->OnPreferencesChanged(GetPreferences());
  this->CreateTemporaryDirectory();
  this->StartDicomDirectoryListener();

  m_Controls->m_ctkDICOMQueryRetrieveWidget->useProgressDialog(true);

  connect(m_Controls->tabWidget, &QTabWidget::currentChanged,
          this, &Self::OnTabChanged);

  connect(m_Controls->externalDataWidget, &QmitkDicomExternalDataWidget::SignalStartDicomImport,
          m_Controls->internalDataWidget, qOverload<const QStringList&>(&QmitkDicomLocalStorageWidget::OnStartDicomImport));

  connect(m_Controls->externalDataWidget, &QmitkDicomExternalDataWidget::SignalDicomToDataManager,
          this, &Self::OnViewButtonAddToDataManager);

  connect(m_Controls->internalDataWidget, &QmitkDicomLocalStorageWidget::SignalFinishedImport,
          this, &Self::OnDicomImportFinished);

  connect(m_Controls->internalDataWidget, &QmitkDicomLocalStorageWidget::SignalDicomToDataManager,
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
    auto storagePort = m_Controls->m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
    auto storageAET = m_Controls->m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();

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
  m_Controls->tabWidget->setCurrentIndex(0);
}

void QmitkDicomBrowser::StartDicomDirectoryListener()
{
  m_DicomDirectoryListener->SetDicomListenerDirectory(m_TempDirectory);
  m_DicomDirectoryListener->SetDicomFolderSuffix(TEMP_DICOM_FOLDER_SUFFIX);

  connect(m_DicomDirectoryListener, &QmitkDicomDirectoryListener::SignalStartDicomImport,
          m_Controls->internalDataWidget, qOverload<const QStringList&>(&QmitkDicomLocalStorageWidget::OnStartDicomImport),
          Qt::DirectConnection);
}

void QmitkDicomBrowser::TestHandler()
{
  m_Handler = new DicomEventHandler();
  m_Handler->SubscribeSlots();
}

void QmitkDicomBrowser::OnViewButtonAddToDataManager(const QHash<QString, QVariant>& eventProperties)
{
  ctkDictionary properties;
  properties["FilesForSeries"] = eventProperties["FilesForSeries"];

  if (eventProperties.contains("Modality"))
    properties["Modality"] = eventProperties["Modality"];

  m_Publisher->PublishSignals(mitk::PluginActivator::getContext());
  m_Publisher->AddSeriesToDataManagerEvent(properties);
}

void QmitkDicomBrowser::StartStoreSCP()
{
  auto storagePort = m_Controls->m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StoragePort"].toString();
  auto storageAET = m_Controls->m_ctkDICOMQueryRetrieveWidget->getServerParameters()["StorageAETitle"].toString();
  m_Builder.AddPort(storagePort)->AddAETitle(storageAET)->AddTransferSyntax()->AddOtherNetworkOptions()->AddMode()->AddOutputDirectory(m_TempDirectory);
  m_StoreSCPLauncher = new QmitkStoreSCPLauncher(&m_Builder);

  connect(m_StoreSCPLauncher, &QmitkStoreSCPLauncher::SignalStatusOfStoreSCP,
          this, &Self::OnStoreSCPStatusChanged);

  connect(m_StoreSCPLauncher, &QmitkStoreSCPLauncher::SignalStartImport,
          m_Controls->internalDataWidget, qOverload<const QStringList&>(&QmitkDicomLocalStorageWidget::OnStartDicomImport));

  connect(m_StoreSCPLauncher, &QmitkStoreSCPLauncher::SignalStoreSCPError,
          m_DicomDirectoryListener, &QmitkDicomDirectoryListener::OnDicomNetworkError,
          Qt::DirectConnection);

  connect(m_StoreSCPLauncher, &QmitkStoreSCPLauncher::SignalStoreSCPError,
          this, &Self::OnDicomNetworkError,
          Qt::DirectConnection);

  m_StoreSCPLauncher->StartStoreSCP();
}

void QmitkDicomBrowser::OnStoreSCPStatusChanged(const QString& status)
{
  m_Controls->StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicombrowser/network-idle_16.png'> " + status);
}

void QmitkDicomBrowser::OnDicomNetworkError(const QString& status)
{
  m_Controls->StoreSCPStatusLabel->setText("<img src=':/org.mitk.gui.qt.dicombrowser/network-error_16.png'> " + status);
}

void QmitkDicomBrowser::StopStoreSCP()
{
  delete m_StoreSCPLauncher;
  m_StoreSCPLauncher = nullptr;
}

void QmitkDicomBrowser::SetPluginDirectory()
{
  m_PluginDirectory = mitk::PluginActivator::getContext()->getDataFile("").absolutePath();
  m_PluginDirectory.append("/database");
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

void QmitkDicomBrowser::OnPreferencesChanged(const mitk::IPreferences* prefs)
{
  this->SetPluginDirectory();

  m_DatabaseDirectory = QString::fromStdString(prefs->Get("default dicom path", m_PluginDirectory.toStdString()));
  m_Controls->internalDataWidget->SetDatabaseDirectory(m_DatabaseDirectory);
}
