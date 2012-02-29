/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPluginActivator.h"
#include "DicomEventHandler.h"
#include <service/event/ctkEventConstants.h>
#include <ctkDictionary.h>
#include <mitklogmacros.h>
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
    mitk::DicomSeriesReader::UidFileNamesMap dicomSeriesMap = mitk::DicomSeriesReader::GetSeries(ctkEvent.getProperty("Path").toString().toStdString());
    mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(dicomSeriesMap[ctkEvent.getProperty("SeriesUID").toString().toStdString()]);
    if (node.IsNull())
    {
        MITK_ERROR << "Could not load series: " << ctkEvent.getProperty("SeriesUID").toString().toStdString();
    }
    else
    {
        node->SetName(ctkEvent.getProperty("SeriesUID").toString().toStdString());
        ctkServiceReference serviceReference =mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
        mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
        storageService->GetActiveDataStorage().GetPointer()->GetDataStorage()->Add(node);
        mitk::RenderingManager::GetInstance()->SetDataStorage(storageService->GetActiveDataStorage().GetPointer()->GetDataStorage());
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();

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