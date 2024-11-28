/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomEventHandler.h"
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

#include <ImporterUtil.h>

#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>

namespace
{
  mitk::IFileReader* GetReader(mitk::FileReaderRegistry& readerRegistry, const mitk::CustomMimeType& mimeType)
  {
    try
    {
      return readerRegistry.GetReaders(mitk::MimeType(mimeType, -1, -1)).at(0);
    }
    catch (const std::out_of_range&)
    {
      mitkThrow() << "Cannot find " << mimeType.GetCategory() << " " << mimeType.GetComment() << " file reader.";
    }
  }

  void AddRTDoseSeriesToDataManager(const QStringList& listOfFilesForSeries, mitk::DataStorage* dataStorage)
  {
    mitk::FileReaderRegistry readerRegistry;
    auto doseReader = GetReader(readerRegistry, mitk::DICOMRTMimeTypes::DICOMRT_DOSE_MIMETYPE());
    doseReader->SetInput(ImporterUtil::getUTF8String(listOfFilesForSeries.front()));
    auto readerOutput = doseReader->Read();

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

        //set some specific colorwash and isoline properties
        bool showColorWashGlobal = prefNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_COLORWASH_ID, true);

        //Set reference dose property
        double referenceDose = prefNode->GetDouble(mitk::RTUIConstants::REFERENCE_DOSE_ID, mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE);

        mitk::ConfigureNodeAsDoseNode(doseImageNode, mitk::GenerateIsoLevels_Virtuos(), referenceDose, showColorWashGlobal);

        dataStorage->Add(doseImageNode);
        mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
      }
    }
  }

  void AddRTStructSeriesToDataManager(const QStringList& listOfFilesForSeries, mitk::DataStorage* dataStorage)
  {
    mitk::FileReaderRegistry readerRegistry;
    auto structReader = GetReader(readerRegistry, mitk::DICOMRTMimeTypes::DICOMRT_STRUCT_MIMETYPE());
    structReader->SetInput(ImporterUtil::getUTF8String(listOfFilesForSeries.front()));
    auto readerOutput = structReader->Read();

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

      mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
    }
  }

  void AddRTPlanSeriesToDataManager(const QStringList& listOfFilesForSeries, mitk::DataStorage* dataStorage)
  {
    mitk::FileReaderRegistry readerRegistry;
    auto planReader = GetReader(readerRegistry, mitk::DICOMRTMimeTypes::DICOMRT_PLAN_MIMETYPE());
    planReader->SetInput(ImporterUtil::getUTF8String(listOfFilesForSeries.front()));
    auto readerOutput = planReader->Read();

    if (!readerOutput.empty())
    {
      //there is no image, only the properties are interesting
      mitk::Image::Pointer planDummyImage = dynamic_cast<mitk::Image*>(readerOutput.front().GetPointer());

      auto planImageNode = mitk::DataNode::New();
      planImageNode->SetData(planDummyImage);
      planImageNode->SetName("RTPlan");

      dataStorage->Add(planImageNode);
    }
  }
}

QmitkDicomEventHandler::QmitkDicomEventHandler()
{
}

QmitkDicomEventHandler::~QmitkDicomEventHandler()
{
}

void QmitkDicomEventHandler::OnSignalAddSeriesToDataManager(const ctkEvent& ctkEvent)
{
  const auto listOfFilesForSeries = ctkEvent.getProperty("FilesForSeries").toStringList();

  if (listOfFilesForSeries.empty())
  {
    MITK_INFO << "No files found for the current series.";
    return;
  }

  auto serviceRef = mitk::PluginActivator::GetContext()->getServiceReference<mitk::IDataStorageService>();
  auto storageService = mitk::PluginActivator::GetContext()->getService<mitk::IDataStorageService>(serviceRef);
  auto dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

  if (ctkEvent.containsProperty("Modality"))
  {
    // For RT data, if the modality tag isn't defined or is "CT", the image is handled like before.
    const auto modality = ctkEvent.getProperty("Modality").toString().toUpper();

    if (modality == "RTDOSE")
    {
      AddRTDoseSeriesToDataManager(listOfFilesForSeries, dataStorage);
      return;
    }
    else if (modality == "RTSTRUCT")
    {
      AddRTStructSeriesToDataManager(listOfFilesForSeries, dataStorage);
      return;
    }
    else if (modality == "RTPLAN")
    {
      AddRTPlanSeriesToDataManager(listOfFilesForSeries, dataStorage);
      return;
    }
  }

  std::vector<std::string> seriesToLoad;
  QStringListIterator it(listOfFilesForSeries);

  while (it.hasNext())
    seriesToLoad.push_back(ImporterUtil::getUTF8String(it.next()));

  auto baseDatas = mitk::IOUtil::Load(seriesToLoad.front());

  for (const auto& data : baseDatas)
  {
    auto node = mitk::DataNode::New();
    node->SetData(data);

    auto nodeName = mitk::DataNode::NO_NAME_VALUE();

    if (auto nameDataProp = data->GetProperty("name"); nameDataProp.IsNotNull())
    {
      // If data has a name property set by reader, use this name.
      nodeName = nameDataProp->GetValueAsString();
    }
    else
    {
      // If the reader didn't specify a name, generate one.
      nodeName = mitk::GenerateNameFromDICOMProperties(node);
    }

    node->SetName(nodeName);

    dataStorage->Add(node);
  }

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
}

void QmitkDicomEventHandler::OnSignalRemoveSeriesFromStorage(const ctkEvent& /*ctkEvent*/)
{
}

void QmitkDicomEventHandler::SubscribeSlots()
{
  auto ref = mitk::PluginActivator::GetContext()->getServiceReference<ctkEventAdmin>();

  if (!ref)
    return;

  auto eventAdmin = mitk::PluginActivator::GetContext()->getService<ctkEventAdmin>(ref);

  ctkDictionary properties;

  properties[ctkEventConstants::EVENT_TOPIC] = "org/mitk/gui/qt/dicom/ADD";
  eventAdmin->subscribeSlot(this, SLOT(OnSignalAddSeriesToDataManager(const ctkEvent&)), properties);

  properties[ctkEventConstants::EVENT_TOPIC] = "org/mitk/gui/qt/dicom/DELETED";
  eventAdmin->subscribeSlot(this, SLOT(OnSignalRemoveSeriesFromStorage(const ctkEvent&)), properties);
}
