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
#include <mitkContourModelSet.h>
#include <mitkCoreServices.h>
#include <mitkDICOMRTMimeTypes.h>
#include <mitkDoseNodeHelper.h>
#include <mitkFileReaderRegistry.h>
#include <mitkIDataStorageService.h>
#include <mitkIOUtil.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkIsoLevelsGenerator.h>
#include <mitkMimeType.h>
#include <mitkPropertyNameHelper.h>
#include <mitkRenderingManager.h>
#include <mitkRTUIConstants.h>

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

  std::vector<mitk::BaseData::Pointer> Read(const std::string& filename, const mitk::CustomMimeType& mimeType)
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
      return {};
    }

    reader->SetInput(filename);
    return reader->Read();
  }

  bool AddRTDoseToDataStorage(const std::string& filename, mitk::DataStorage* dataStorage)
  {
    auto readerOutput = Read(filename, mitk::DICOMRTMimeTypes::DICOMRT_DOSE_MIMETYPE());

    if (!readerOutput.empty())
    {
      mitk::Image::Pointer doseImage = dynamic_cast<mitk::Image*>(readerOutput.front().GetPointer());

      auto doseImageNode = mitk::DataNode::New();
      doseImageNode->SetData(doseImage);
      doseImageNode->SetName("RTDose");

      if (doseImage.IsNotNull())
      {
        std::string sopUID;

        if (mitk::GetBackwardsCompatibleDICOMProperty(0x0008, 0x0016, "dicomseriesreader.SOPClassUID", doseImage->GetPropertyList(), sopUID))
          doseImageNode->SetName(sopUID);

        auto prefService = mitk::CoreServices::GetPreferencesService();
        auto prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

        if (prefNode == nullptr)
          mitkThrow() << "Error in preference interface. Cannot find preset node with name \"" << prefNode << '"';

        double referenceDose = prefNode->GetDouble(mitk::RTUIConstants::REFERENCE_DOSE_ID, mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE);
        bool showColorWashGlobal = prefNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_COLORWASH_ID, true);

        mitk::ConfigureNodeAsDoseNode(doseImageNode, mitk::GenerateIsoLevels_Virtuos(), referenceDose, showColorWashGlobal);

        dataStorage->Add(doseImageNode);
        return true;
      }
    }

    return false;
  }

  bool AddRTStructToDataStorage(const std::string& filename, mitk::DataStorage* dataStorage)
  {
    auto readerOutput = Read(filename, mitk::DICOMRTMimeTypes::DICOMRT_STRUCT_MIMETYPE());

    if (readerOutput.empty())
    {
      MITK_ERROR << "No structure sets were created";
    }
    else
    {
      for (const auto& aStruct : readerOutput)
      {
        mitk::ContourModelSet::Pointer countourModelSet = dynamic_cast<mitk::ContourModelSet*>(aStruct.GetPointer());

        auto structNode = mitk::DataNode::New();
        structNode->SetData(countourModelSet);
        structNode->SetProperty("name", aStruct->GetProperty("name"));
        structNode->SetProperty("color", aStruct->GetProperty("contour.color"));
        structNode->SetProperty("contour.color", aStruct->GetProperty("contour.color"));
        structNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));

        dataStorage->Add(structNode);
      }

      return true;
    }

    return false;
  }

  void AddRTPlanToDataStorage(const std::string& filename, mitk::DataStorage* dataStorage)
  {
    auto readerOutput = Read(filename, mitk::DICOMRTMimeTypes::DICOMRT_PLAN_MIMETYPE());

    if (!readerOutput.empty())
    {
      // There is no image, only the properties are of interest.
      mitk::Image::Pointer planDummyImage = dynamic_cast<mitk::Image*>(readerOutput.front().GetPointer());

      auto planImageNode = mitk::DataNode::New();
      planImageNode->SetData(planDummyImage);
      planImageNode->SetName("RTPlan");

      dataStorage->Add(planImageNode);
    }
  }
}

const std::string QmitkDicomBrowser::EDITOR_ID = "org.mitk.editors.dicombrowser";

QmitkDicomBrowser::QmitkDicomBrowser()
  : m_Ui(new Ui::QmitkDicomBrowser)
{
}

QmitkDicomBrowser::~QmitkDicomBrowser()
{
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
  auto serviceRef = mitk::PluginActivator::GetContext()->getServiceReference<mitk::IDataStorageService>();
  auto storageService = mitk::PluginActivator::GetContext()->getService<mitk::IDataStorageService>(serviceRef);
  auto dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

  bool reinit = false;

  for (const auto& [filename, modality] : series)
  {
    if (modality.has_value())
    {
      if (modality == "RTDOSE")
      {
        reinit |= AddRTDoseToDataStorage(filename, dataStorage);
        continue;
      }
      else if (modality == "RTSTRUCT")
      {
        reinit |= AddRTStructToDataStorage(filename, dataStorage);
        continue;
      }
      else if (modality == "RTPLAN")
      {
        AddRTPlanToDataStorage(filename, dataStorage);
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

    if (reinit)
      mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
  }
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
