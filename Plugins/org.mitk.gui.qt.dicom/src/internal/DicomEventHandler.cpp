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
#include <QmitkBaseFunctionalityComponent.h>
#include <mitkIDataStorageService.h>
#include <service/event/ctkEventAdmin.h>
#include <ctkServiceReference.h>
#include <mitkRenderingManager.h>


DicomEventHandler::DicomEventHandler() 
{
}

DicomEventHandler::~DicomEventHandler()
{
}

void DicomEventHandler::OnSignalAddSeriesToDataManager(const ctkEvent& ctkEvent)
{
    QString patientName = ctkEvent.getProperty("PatientName").toString();
    QString studyUID = ctkEvent.getProperty("StudyUID").toString();
    QString studyName = ctkEvent.getProperty("StudyName").toString();
    QString seriesUID = ctkEvent.getProperty("SeriesUID").toString();
    QString seriesName = ctkEvent.getProperty("SeriesName").toString();
    QString path = ctkEvent.getProperty("Path").toString(); 

    std::list<std::string> qualifiedUIDs;
    mitk::DicomSeriesReader::StringContainer seriesToLoad;
    std::size_t found;

    mitk::DicomSeriesReader::UidFileNamesMap dicomSeriesMap = mitk::DicomSeriesReader::GetSeries(path.toStdString(),false);
    mitk::DicomSeriesReader::UidFileNamesMap::const_iterator qualifiedSeriesInstanceUIDIterator;

    for(qualifiedSeriesInstanceUIDIterator = dicomSeriesMap.begin();
        qualifiedSeriesInstanceUIDIterator != dicomSeriesMap.end();
        ++qualifiedSeriesInstanceUIDIterator)
    {
        found = qualifiedSeriesInstanceUIDIterator->first.find(seriesUID.toStdString());
        if(found!= qualifiedSeriesInstanceUIDIterator->first.npos)
        {
            qualifiedUIDs.push_back(qualifiedSeriesInstanceUIDIterator->first); 
            seriesToLoad = qualifiedSeriesInstanceUIDIterator->second;
        }
    }

    mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(seriesToLoad);
    if (node.IsNull())
    {
        MITK_ERROR << "Could not load series: " << seriesUID.toStdString();
    }
    else
    {
        //Get Reference for default data storage.
        ctkServiceReference serviceReference =mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
        mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
        mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

        dataStorage->Add(node);

        // Initialize the RenderWindow
        mitk::TimeSlicedGeometry::Pointer geometry = dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll());
        mitk::RenderingManager::GetInstance()->InitializeViews(geometry);


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
