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

#include "mitkPluginActivator.h"
#include "DicomEventHandler.h"
#include <service/event/ctkEventConstants.h>
#include <ctkDictionary.h>
#include <mitkLogMacros.h>
#include <mitkDataNode.h>
#include <mitkIDataStorageService.h>
#include <service/event/ctkEventAdmin.h>
#include <ctkServiceReference.h>
#include <mitkRenderingManager.h>
#include <QVector>
#include "mitkImage.h"
#include <mitkContourModelSet.h>

#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMEnums.h>
#include <mitkDICOMTagsOfInterestHelper.h>
#include <mitkDICOMProperty.h>
#include <mitkStringProperty.h>
#include <mitkDicomSeriesReader.h>

#include <mitkRTDoseReader.h>
#include <mitkRTPlanReader.h>
#include <mitkRTStructureSetReader.h>
#include <mitkRTConstants.h>
#include <mitkIsoDoseLevelCollections.h>
#include <mitkIsoDoseLevelSetProperty.h>
#include <mitkIsoDoseLevelVectorProperty.h>
#include <mitkDoseImageVtkMapper2D.h>
#include <mitkRTUIConstants.h>
#include <mitkIsoLevelsGenerator.h>

#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkRenderingModeProperty.h>
#include <mitkLocaleSwitch.h>

#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryPlatform.h>

DicomEventHandler::DicomEventHandler()
{
}

DicomEventHandler::~DicomEventHandler()
{
}

void DicomEventHandler::OnSignalAddSeriesToDataManager(const ctkEvent& ctkEvent)
{
  QStringList listOfFilesForSeries;
  listOfFilesForSeries = ctkEvent.getProperty("FilesForSeries").toStringList();

  if (!listOfFilesForSeries.isEmpty())
  {
    //for rt data, if the modality tag isn't defined or is "CT" the image is handled like before
    if(ctkEvent.containsProperty("Modality") &&
       (ctkEvent.getProperty("Modality").toString().compare("RTDOSE",Qt::CaseInsensitive) == 0 ||
        ctkEvent.getProperty("Modality").toString().compare("RTSTRUCT",Qt::CaseInsensitive) == 0 ||
        ctkEvent.getProperty("Modality").toString().compare("RTPLAN", Qt::CaseInsensitive) == 0))
    {
      QString modality = ctkEvent.getProperty("Modality").toString();

      if(modality.compare("RTDOSE",Qt::CaseInsensitive) == 0)
      {
          auto doseReader = mitk::RTDoseReader();
          doseReader.SetInput(listOfFilesForSeries.front().toStdString());
          std::vector<itk::SmartPointer<mitk::BaseData> > readerOutput = doseReader.Read();
          if (!readerOutput.empty()){
            mitk::Image::Pointer doseImage = dynamic_cast<mitk::Image*>(readerOutput.at(0).GetPointer());

            mitk::DataNode::Pointer doseImageNode = mitk::DataNode::New();
            doseImageNode->SetData(doseImage);
            doseImageNode->SetName("RTDose");

            mitk::DataNode::Pointer doseOutlineNode = mitk::DataNode::New();

            doseOutlineNode->SetData(doseImage);
            if (doseImage != nullptr)
            {
                auto sopUIDProperty = doseImage->GetProperty("dicomseriesreader.SOPClassUID");
                if (sopUIDProperty.IsNotNull()){
                    auto sopUIDStringProperty = dynamic_cast<mitk::StringProperty*>(sopUIDProperty.GetPointer());
                    if (sopUIDStringProperty != nullptr){
                        std::string sopUID = sopUIDStringProperty->GetValue();
                        doseImageNode->SetName(sopUID);
                    }
                }

                berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
                berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID.c_str());

                typedef QStringList NamesType;
                NamesType names = prefNode->ChildrenNames();

                std::map<std::string, mitk::IsoDoseLevelSet::Pointer> presetMap;

                for (NamesType::const_iterator pos = names.begin(); pos != names.end(); ++pos)
                {
                    berry::IPreferences::Pointer aPresetNode = prefNode->Node(*pos);

                    if (aPresetNode.IsNull())
                    {
                        mitkThrow() << "Error in preference interface. Cannot find preset node under given name. Name: " << (*pos).toStdString();
                    }

                    mitk::IsoDoseLevelSet::Pointer levelSet = mitk::IsoDoseLevelSet::New();

                    NamesType levelNames = aPresetNode->ChildrenNames();
                    for (NamesType::const_iterator levelName = levelNames.begin(); levelName != levelNames.end(); ++levelName)
                    {
                        berry::IPreferences::Pointer levelNode = aPresetNode->Node(*levelName);
                        if (aPresetNode.IsNull())
                        {
                            mitkThrow() << "Error in preference interface. Cannot find level node under given preset name. Name: " << (*pos).toStdString() << "; Level id: " << (*levelName).toStdString();
                        }

                        mitk::IsoDoseLevel::Pointer isoLevel = mitk::IsoDoseLevel::New();

                        isoLevel->SetDoseValue(levelNode->GetDouble(mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID.c_str(), 0.0));
                        mitk::IsoDoseLevel::ColorType color;
                        color.SetRed(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID.c_str(), 1.0));
                        color.SetGreen(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID.c_str(), 1.0));
                        color.SetBlue(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID.c_str(), 1.0));
                        isoLevel->SetColor(color);
                        isoLevel->SetVisibleIsoLine(levelNode->GetBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID.c_str(), true));
                        isoLevel->SetVisibleColorWash(levelNode->GetBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID.c_str(), true));

                        levelSet->SetIsoDoseLevel(isoLevel);
                    }

                    presetMap.insert(std::make_pair((*pos).toStdString(), levelSet));
                }

                if (presetMap.size() == 0)
                {
                    presetMap.insert(std::make_pair(std::string("Virtuos"), mitk::GeneratIsoLevels_Virtuos()));
                }


                prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

                if (prefNode.IsNull())
                {
                    mitkThrow() << "Error in preference interface. Cannot find preset node under given name. Name: " << prefNode->ToString().toStdString();
                }

                //set some specific colorwash and isoline properties
                bool showColorWashGlobal = prefNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_COLORWASH_ID.c_str(), true);
                doseImageNode->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), showColorWashGlobal);

                bool showIsolinesGlobal = prefNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_ISOLINES_ID.c_str(), true);
                doseOutlineNode->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), showIsolinesGlobal);

                //Set reference dose property
                double referenceDose = prefNode->GetDouble(mitk::RTUIConstants::REFERENCE_DOSE_ID.c_str(), mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE);
                doseImageNode->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);
                doseOutlineNode->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);

                QString presetName = prefNode->Get(mitk::RTUIConstants::SELECTED_ISO_PRESET_ID.c_str(), "Virtuos");

                mitk::IsoDoseLevelSet::Pointer isoDoseLevelPreset = presetMap[presetName.toStdString()];
                mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(isoDoseLevelPreset);

                doseImageNode->SetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(), levelSetProp);
                doseOutlineNode->SetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(), levelSetProp);

                mitk::IsoDoseLevelVector::Pointer levelVector = mitk::IsoDoseLevelVector::New();
                mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(levelVector);
                doseImageNode->SetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(), levelVecProp);
                doseOutlineNode->SetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(), levelVecProp);

                mitk::RenderingModeProperty::Pointer renderingModeProp = mitk::RenderingModeProperty::New();

                if (showColorWashGlobal)
                {
                    //Generating the Colorwash
                    vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

                    for (mitk::IsoDoseLevelSet::ConstIterator itIsoDoseLevel = isoDoseLevelPreset->Begin(); itIsoDoseLevel != isoDoseLevelPreset->End(); ++itIsoDoseLevel)
                    {
                        float *hsv = new float[3];
                        //used for transfer rgb to hsv
                        vtkSmartPointer<vtkMath> cCalc = vtkSmartPointer<vtkMath>::New();
                        if (itIsoDoseLevel->GetVisibleColorWash()){
                            cCalc->RGBToHSV(itIsoDoseLevel->GetColor()[0], itIsoDoseLevel->GetColor()[1], itIsoDoseLevel->GetColor()[2], &hsv[0], &hsv[1], &hsv[2]);
                            transferFunction->AddHSVPoint(itIsoDoseLevel->GetDoseValue()*referenceDose, hsv[0], hsv[1], hsv[2], 1.0, 1.0);
                        }
                    }

                    mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
                    mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
                    mitkTransFunc->SetColorTransferFunction(transferFunction);
                    mitkTransFuncProp->SetValue(mitkTransFunc);
                    doseImageNode->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);


                    renderingModeProp->SetValue(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);
                }
                else
                {
                    //Set rendering mode to levelwindow color mode
                    renderingModeProp->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
                }

                doseImageNode->SetProperty("Image Rendering.Mode", renderingModeProp);
                doseImageNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));

                //set the outline properties
                doseOutlineNode->SetBoolProperty("outline binary", true);
                doseOutlineNode->SetProperty("helper object", mitk::BoolProperty::New(true));
                doseOutlineNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));

                ctkServiceReference serviceReference = mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
                mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
                mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

                dataStorage->Add(doseImageNode);
                dataStorage->Add(doseOutlineNode, doseImageNode);

                //set the dose mapper for outline drawing; the colorwash is realized by the imagevtkmapper2D
                mitk::DoseImageVtkMapper2D::Pointer contourMapper = mitk::DoseImageVtkMapper2D::New();
                doseOutlineNode->SetMapper(1, contourMapper);

                mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
            }
        }//END DOSE
      }
      else if(modality.compare("RTSTRUCT",Qt::CaseInsensitive) == 0)
      {
          auto structReader = mitk::RTStructureSetReader();
          structReader.SetInput(listOfFilesForSeries.front().toStdString());
          std::vector<itk::SmartPointer<mitk::BaseData> > readerOutput = structReader.Read();

          if (readerOutput.empty()){
              MITK_ERROR << "No structure sets were created" << endl;
          }
          else {
              std::vector<mitk::DataNode::Pointer> modelVector;

              ctkServiceReference serviceReference = mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
              mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
              mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

              for (const auto& aStruct : readerOutput){
                  mitk::ContourModelSet::Pointer countourModelSet = dynamic_cast<mitk::ContourModelSet*>(aStruct.GetPointer());

                  mitk::DataNode::Pointer structNode = mitk::DataNode::New();
                  structNode->SetData(countourModelSet);
                  structNode->SetProperty("name", aStruct->GetProperty("name"));
                  structNode->SetProperty("color", aStruct->GetProperty("contour.color"));
                  structNode->SetProperty("contour.color", aStruct->GetProperty("contour.color"));
                  structNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
                  structNode->SetVisibility(true, mitk::BaseRenderer::GetInstance(
                          mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")));
                  structNode->SetVisibility(false, mitk::BaseRenderer::GetInstance(
                      mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")));
                  structNode->SetVisibility(false, mitk::BaseRenderer::GetInstance(
                      mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")));
                  structNode->SetVisibility(true, mitk::BaseRenderer::GetInstance(
                      mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

                  dataStorage->Add(structNode);
              }
              mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
          }
      }
      else if (modality.compare("RTPLAN", Qt::CaseInsensitive) == 0)
      {
          auto planReader = mitk::RTPlanReader();
          planReader.SetInput(listOfFilesForSeries.front().toStdString());
          std::vector<itk::SmartPointer<mitk::BaseData> > readerOutput = planReader.Read();
          if (!readerOutput.empty()){
              //there is no image, only the properties are interesting
              mitk::Image::Pointer planDummyImage = dynamic_cast<mitk::Image*>(readerOutput.at(0).GetPointer());

              mitk::DataNode::Pointer planImageNode = mitk::DataNode::New();
              planImageNode->SetData(planDummyImage);
              planImageNode->SetName("RTPlan");

              ctkServiceReference serviceReference = mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
              mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
              mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

              dataStorage->Add(planImageNode);
          }
      }
    }
    else
    {
      mitk::StringList seriesToLoad;
      QStringListIterator it(listOfFilesForSeries);

      while (it.hasNext())
      {
        seriesToLoad.push_back(it.next().toStdString());
      }

      //Get Reference for default data storage.
      ctkServiceReference serviceReference = mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
      mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
      mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

      //special handling of Philips 3D US DICOM.
      //Copied from DICOMSeriesReaderService
      if (!seriesToLoad.empty() && mitk::DicomSeriesReader::IsPhilips3DDicom(seriesToLoad.front()))
      {
          MITK_INFO << "it is a Philips3D US Dicom file" << std::endl;
          mitk::LocaleSwitch localeSwitch("C");
          std::locale previousCppLocale(std::cin.getloc());
          std::locale l("C");
          std::cin.imbue(l);

          mitk::DataNode::Pointer node = mitk::DataNode::New();
          mitk::DicomSeriesReader::StringContainer stringvec;
          stringvec.push_back(seriesToLoad.front());
          if (mitk::DicomSeriesReader::LoadDicomSeries(stringvec, *node))
          {
              mitk::BaseData::Pointer data = node->GetData();
              mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New(itksys::SystemTools::GetFilenameName(seriesToLoad.front()));
              data->GetPropertyList()->SetProperty("name", nameProp);
              node->SetProperty("name", nameProp);
              dataStorage->Add(node);
          }
          std::cin.imbue(previousCppLocale);
          return;
      }

      //Normal DICOM handling (It wasn't a Philips 3D US)
      mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();

      selector->LoadBuiltIn3DConfigs();
      selector->LoadBuiltIn3DnTConfigs();
      selector->SetInputFiles(seriesToLoad);

      mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();
      reader->SetAdditionalTagsOfInterest(mitk::GetCurrentDICOMTagsOfInterest());
      reader->SetTagLookupTableToPropertyFunctor(mitk::GetDICOMPropertyForDICOMValuesFunctor);
      reader->SetInputFiles(seriesToLoad);

      mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
      scanner->AddTagPaths(reader->GetTagsOfInterest());
      scanner->SetInputFiles(seriesToLoad);
      scanner->Scan();

      reader->SetTagCache(scanner->GetScanCache());
      reader->AnalyzeInputFiles();
      reader->LoadImages();

      for (unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i)
      {
          const mitk::DICOMImageBlockDescriptor& desc = reader->GetOutput(i);
          mitk::BaseData::Pointer data = desc.GetMitkImage().GetPointer();

          std::string nodeName = "Unnamed_DICOM";

          std::string studyDescription = desc.GetPropertyAsString("studyDescription");
          std::string seriesDescription = desc.GetPropertyAsString("seriesDescription");

          if (!studyDescription.empty())
          {
              nodeName = studyDescription;
          }

          if (!seriesDescription.empty())
          {
              if (!studyDescription.empty())
              {
                  nodeName += "/";
              }
              nodeName += seriesDescription;
          }

          mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New(nodeName);
          data->SetProperty("name", nameProp);

          mitk::DataNode::Pointer node = mitk::DataNode::New();
          node->SetData(data);
          nameProp = mitk::StringProperty::New(nodeName);
          node->SetProperty("name", nameProp);

          dataStorage->Add(node);
      }

      if (reader->GetNumberOfOutputs() < 1)
      {
          MITK_ERROR << "Error loading series: " << ctkEvent.getProperty("SeriesName").toString().toStdString()
              << " id: " << ctkEvent.getProperty("SeriesUID").toString().toStdString();
      }
    }
  }
  else
  {
    MITK_INFO << "There are no files for the current series";
  }
}

void DicomEventHandler::OnSignalRemoveSeriesFromStorage(const ctkEvent& /*ctkEvent*/)
{
}

void DicomEventHandler::SubscribeSlots()
{
  ctkServiceReference ref = mitk::PluginActivator::getContext()->getServiceReference<ctkEventAdmin>();
  if (ref)
  {
    ctkEventAdmin* eventAdmin = mitk::PluginActivator::getContext()->getService<ctkEventAdmin>(ref);
    ctkDictionary properties;
    properties[ctkEventConstants::EVENT_TOPIC] = "org/mitk/gui/qt/dicom/ADD";
    eventAdmin->subscribeSlot(this, SLOT(OnSignalAddSeriesToDataManager(ctkEvent)), properties);
    properties[ctkEventConstants::EVENT_TOPIC] = "org/mitk/gui/qt/dicom/DELETED";
    eventAdmin->subscribeSlot(this, SLOT(OnSignalRemoveSeriesFromStorage(ctkEvent)), properties);
  }
}
