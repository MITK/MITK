/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomBrowser.h"
#include <ui_QmitkDicomBrowser.h>

#include "mitkPluginActivator.h"

#include <mitkBaseDICOMReaderService.h>
#include <mitkCoreServices.h>
#include <mitkDICOMRTMimeTypes.h>
#include <mitkFileReaderRegistry.h>
#include <mitkIDataStorageService.h>
#include <mitkIOUtil.h>
#include <mitkMimeType.h>
#include <mitkRenderingManager.h>

#include <QMessageBox>

namespace
{
  void GenerateNodeName(mitk::DataNode* node)
  {
    if (auto nameProperty = node->GetData()->GetProperty("name"); nameProperty.IsNotNull())
    {
      // If data has a name property set by reader, use this name.
      node->SetName(nameProperty->GetValueAsString());
    }
    else
    {
      // If the reader didn't specify a name, generate one.
      node->SetName(mitk::GenerateNameFromDICOMProperties(node));
    }
  }

  bool ReadIntoDataStorage(const std::string& filename, const mitk::CustomMimeType& mimeType, mitk::DataStorage* dataStorage)
  {
    mitk::FileReaderRegistry readerRegistry;
    mitk::IFileReader* reader = nullptr;

    try
    {
      reader = readerRegistry.GetReaders(mitk::MimeType(mimeType, -1, -1)).at(0);
    }
    catch (const std::out_of_range&)
    {
      const std::string message = "Cannot find " + mimeType.GetCategory() + "( " + mimeType.GetComment() + ") file reader.";
      MITK_ERROR << message;
      QMessageBox::critical(nullptr, "View DICOM series", QString::fromStdString(message));
      return false;
    }

    reader->SetInput(filename);

    try
    {
      return !reader->Read(*dataStorage)->empty();
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << e.what();
      QMessageBox::critical(nullptr, "View DICOM series", e.what());
      return false;
    }
  }
}

const std::string QmitkDicomBrowser::EDITOR_ID = "org.mitk.editors.dicombrowser";

QmitkDicomBrowser::QmitkDicomBrowser()
  : m_Ui(std::make_unique<Ui::QmitkDicomBrowser>())
{
}

QmitkDicomBrowser::~QmitkDicomBrowser()
{
  // The ctkDICOMQueryRetrieveWidget creates an in-memory database for query results.
  // However, a ctkDICOMTagCache.sql buddy file is still created on disk in the
  // working directory. Until it is fixed in CTK, we manually remove the temporary
  // file.

  QFile queryTagCacheFile(QDir().currentPath() + "/ctkDICOMTagCache.sql");

  if (queryTagCacheFile.exists())
    queryTagCacheFile.remove();

}

void QmitkDicomBrowser::CreateQtPartControl(QWidget *parent)
{
  using Self = QmitkDicomBrowser;

  m_Ui->setupUi(parent);

  auto databaseDir = mitk::PluginActivator::GetContext()->getDataFile("").absolutePath().append("/database");
  auto database = m_Ui->localStorageWidget->SetDatabaseDirectory(databaseDir);

  m_Ui->queryRetrieveWidget->setRetrieveDatabase(database);
  m_Ui->queryRetrieveWidget->useProgressDialog(true);

  connect(m_Ui->importWidget, &QmitkDicomImportWidget::Import,
          m_Ui->localStorageWidget, &QmitkDicomLocalStorageWidget::OnImport);

  connect(m_Ui->importWidget, &QmitkDicomImportWidget::ViewSeries,
          this, &Self::OnViewSeries);

  connect(m_Ui->localStorageWidget, &QmitkDicomLocalStorageWidget::IndexingComplete,
          this, &Self::OnIndexingComplete);

  connect(m_Ui->localStorageWidget, &QmitkDicomLocalStorageWidget::ViewSeries,
          this, &Self::OnViewSeries);

  m_Ui->tabWidget->setCurrentIndex(database->seriesCount() > 0
    ? Tabs::LocalStorage
    : Tabs::Import);
}

void QmitkDicomBrowser::OnIndexingComplete()
{
  m_Ui->tabWidget->setCurrentIndex(Tabs::LocalStorage);
}

void QmitkDicomBrowser::OnViewSeries(const std::vector<std::pair<std::string, std::optional<std::string>>>& series)
{
  mitk::CoreServicePointer<mitk::IDataStorageService> dsService(mitk::CoreServices::GetDataStorageService());
  if (!dsService)
  {
    MITK_ERROR << "IDataStorageService not available. Unable to load DICOM data.";
    QMessageBox::critical(nullptr, "View DICOM series", "Data storage service not available.");
    return;
  }

  auto dataStorage = dsService->GetDefaultDataStorage().GetPointer();

  bool reinit = false;

  for (const auto& [filename, modality] : series)
  {
    if (modality.has_value())
    {
      if (modality == "RTDOSE")
      {
        reinit |= ReadIntoDataStorage(filename, mitk::DICOMRTMimeTypes::DICOMRT_DOSE_MIMETYPE(), dataStorage);
        continue;
      }
      else if (modality == "RTSTRUCT")
      {
        reinit |= ReadIntoDataStorage(filename, mitk::DICOMRTMimeTypes::DICOMRT_STRUCT_MIMETYPE(), dataStorage);
        continue;
      }
      else if (modality == "RTPLAN")
      {
        reinit |= ReadIntoDataStorage(filename, mitk::DICOMRTMimeTypes::DICOMRT_PLAN_MIMETYPE(), dataStorage);
        continue;
      }
    }

    std::vector<mitk::BaseData::Pointer> baseDatas;

    try
    {
      baseDatas = mitk::IOUtil::Load(filename);
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << e.GetDescription();
      QMessageBox::critical(nullptr, "View DICOM series", e.GetDescription());
    }

    for (const auto& data : baseDatas)
    {
      auto node = mitk::DataNode::New();
      node->SetData(data);

      GenerateNodeName(node);

      dataStorage->Add(node);
      reinit = true;
    }
  }

  if (reinit)
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
}

void QmitkDicomBrowser::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  this->SetSite(site);
  this->SetInput(input);
}

void QmitkDicomBrowser::SetFocus()
{
}

void QmitkDicomBrowser::DoSave()
{
}

void QmitkDicomBrowser::DoSaveAs()
{
}

bool QmitkDicomBrowser::IsDirty() const
{
  return false;
}

bool QmitkDicomBrowser::IsSaveAsAllowed() const
{
  return false;
}

berry::IPartListener::Events::Types QmitkDicomBrowser::GetPartEventTypes() const
{
  return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}
