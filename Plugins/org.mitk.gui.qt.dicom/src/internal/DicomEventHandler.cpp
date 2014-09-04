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
#include <mitkDicomSeriesReader.h>
#include <mitkDataNode.h>
#include <mitkIDataStorageService.h>
#include <service/event/ctkEventAdmin.h>
#include <ctkServiceReference.h>
#include <mitkRenderingManager.h>
#include <QVector>
#include "mitkImage.h"
#include <mitkContourModelSet.h>

#include <mitkRTDoseReader.h>
#include <mitkRTStructureSetReader.h>
#include <mitkRTConstants.h>

DicomEventHandler::DicomEventHandler()
{
}

DicomEventHandler::~DicomEventHandler()
{
}

void DicomEventHandler::OnSignalAddSeriesToDataManager(const ctkEvent& ctkEvent)
{
  QStringList listOfFilesForSeries;
  mitk::DicomSeriesReader::StringContainer seriesToLoad;

  listOfFilesForSeries = ctkEvent.getProperty("FilesForSeries").toStringList();

  if (!listOfFilesForSeries.isEmpty()){

    //for rt data, if the modality tag isnt defined or is "CT" the image is handled like before
    if(ctkEvent.containsProperty("Modality") &&
       (ctkEvent.getProperty("Modality").toString().compare("RTDOSE",Qt::CaseInsensitive) == 0 ||
        ctkEvent.getProperty("Modality").toString().compare("RTSTRUCT",Qt::CaseInsensitive) == 0))
    {
      QString modality = ctkEvent.getProperty("Modality").toString();
      MITK_INFO << endl << "----> MODALITY: " << ctkEvent.getProperty("Modality").toString().toStdString() << endl;

      if(modality.compare("RTDOSE",Qt::CaseInsensitive) == 0)
      {
        //        MITK_INFO << "------->FILEname: " << listOfFilesForSeries.at(0).toStdString() << endl;
//        mitk::RTDoseReader::Pointer doseReader = mitk::RTDoseReader::New();

//        mitk::DataNode::Pointer doseImageNode = mitk::DataNode::New();
//        mitk::DataNode::Pointer doseOutlineNode = mitk::DataNode::New();

//        doseImageNode = doseReader->LoadRTDose(listOfFilesForSeries.at(0).toStdString().c_str());
//        doseOutlineNode->SetData(doseImageNode->GetData());
//        if(doseImageNode.IsNotNull() && doseOutlineNode->GetData() != NULL)
//        {
//          double referenceDose = 40.0;
//          mitk::GetReferenceDoseValue(referenceDose);
//          //set some specific colorwash and isoline properties
//          doseImageNode->SetBoolProperty(mitk::Constants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), true);
//          doseOutlineNode->SetBoolProperty(mitk::Constants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), true);
//          //Set reference dose property
//          doseImageNode->SetFloatProperty(mitk::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);
//          doseOutlineNode->SetFloatProperty(mitk::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);

//          //Get the presets and put it into properties for the datanodes
//          mitk::PresetMapType presetsMap = mitk::rt::LoadPresetsMap();
//          std::string selectedPresetName = mitk::rt::GetSelectedPresetName();

//          mitk::IsoDoseLevelSet::Pointer isoDoseLevelPreset = presetsMap[mitk::rt::GetSelectedPresetName()];
//          mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(isoDoseLevelPreset);
//          doseImageNode->SetProperty(mitk::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),levelSetProp);
//          doseOutlineNode->SetProperty(mitk::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),levelSetProp);

//          mitk::IsoDoseLevelVector::Pointer levelVector = mitk::IsoDoseLevelVector::New();
//          mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(levelVector);
//          doseImageNode->SetProperty(mitk::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),levelVecProp);
//          doseOutlineNode->SetProperty(mitk::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),levelVecProp);

//          //Generating the Colorwash
//          vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

//          for(mitk::IsoDoseLevelSet::ConstIterator itIsoDoseLevel = isoDoseLevelPreset->Begin(); itIsoDoseLevel != isoDoseLevelPreset->End(); ++itIsoDoseLevel)
//          {
//            float *hsv = new float[3];
//            //used for transfer rgb to hsv
//            vtkSmartPointer<vtkMath> cCalc = vtkSmartPointer<vtkMath>::New();
//            if(itIsoDoseLevel->GetVisibleColorWash()){
//              cCalc->RGBToHSV(itIsoDoseLevel->GetColor()[0],itIsoDoseLevel->GetColor()[1],itIsoDoseLevel->GetColor()[2],&hsv[0],&hsv[1],&hsv[2]);
//              transferFunction->AddHSVPoint(itIsoDoseLevel->GetDoseValue()*referenceDose,hsv[0],hsv[1],hsv[2],1.0,1.0);
//            }
//          }

//          mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
//          mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
//          mitkTransFunc->SetColorTransferFunction(transferFunction);
//          mitkTransFuncProp->SetValue(mitkTransFunc);

//          mitk::RenderingModeProperty::Pointer renderingModeProp = mitk::RenderingModeProperty::New();
//          renderingModeProp->SetValue(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);

//          doseImageNode->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);
//          doseImageNode->SetProperty("Image Rendering.Mode", renderingModeProp);
//          doseImageNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));

//          //set the outline properties
//          doseOutlineNode->SetBoolProperty("outline binary", true);
//          doseOutlineNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
//          doseOutlineNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false) );

//          GetDataStorage()->Add(doseImageNode);
//          GetDataStorage()->Add(doseOutlineNode);

//          //set the dose mapper for outline drawing; the colorwash is realized by the imagevtkmapper2D
//          mitk::DoseImageVtkMapper2D::Pointer contourMapper = mitk::DoseImageVtkMapper2D::New();
//          doseOutlineNode->SetMapper(1,contourMapper);

//          mitk::TimeGeometry::Pointer geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
//          mitk::RenderingManager::GetInstance()->InitializeViews( geo );
//        }
      }
      else if(modality.compare("RTSTRUCT",Qt::CaseInsensitive) == 0)
      {
        mitk::RTStructureSetReader::Pointer structreader = mitk::RTStructureSetReader::New();
        std::deque<mitk::DataNode::Pointer> modelVector = structreader->ReadStructureSet(listOfFilesForSeries.at(0).toStdString().c_str());

        if(modelVector.empty())
        {
          MITK_ERROR << "No structuresets were created" << endl;
        }
        else
        {
          ctkServiceReference serviceReference =mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
          mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
          mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

          for(int i=0; i<modelVector.size();i++)
          {
            dataStorage->Add(modelVector.at(i));
          }
          mitk::TimeGeometry::Pointer geometry = dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll());
          mitk::RenderingManager::GetInstance()->InitializeViews(geometry);
        }
      }
    }
    else
    {

      QStringListIterator it(listOfFilesForSeries);

      while (it.hasNext())
      {
        seriesToLoad.push_back(it.next().toStdString());
      }


      mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(seriesToLoad);
      if (node.IsNull())
      {
        MITK_ERROR << "Error loading series: " << ctkEvent.getProperty("SeriesName").toString().toStdString()
                   << " id: " <<ctkEvent.getProperty("SeriesUID").toString().toStdString();
      }
      else
      {
        //Get Reference for default data storage.
        ctkServiceReference serviceReference =mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
        mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
        mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

        dataStorage->Add(node);

        // Initialize the RenderWindow
        mitk::TimeGeometry::Pointer geometry = dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geometry);
      }
    }
  }
  else
  {
    MITK_INFO << "There are no files for the current series";
  }
}

void DicomEventHandler::OnSignalRemoveSeriesFromStorage(const ctkEvent& ctkEvent)
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
