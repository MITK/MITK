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
#include "QmitkDicomDataEventPublisher.h"

#include <ctkServiceReference.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>

QmitkDicomDataEventPublisher::QmitkDicomDataEventPublisher()
{
}

QmitkDicomDataEventPublisher::~QmitkDicomDataEventPublisher()
{
}

void QmitkDicomDataEventPublisher::AddSeriesToDataManagerEvent(const ctkDictionary& properties)
{
    emit SignalAddSeriesToDataManager(properties);
}

void QmitkDicomDataEventPublisher::RemoveSeriesFromStorageEvent(const ctkDictionary& properties)
{
    emit SignalRemoveSeriesFromStorage(properties);
}

void QmitkDicomDataEventPublisher::PublishSignals(ctkPluginContext* context)
{
    ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
    if (ref)
    {
      ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
      // Using Qt::DirectConnection is equivalent to ctkEventAdmin::sendEvent()
      eventAdmin->publishSignal(this, SIGNAL(SignalAddSeriesToDataManager(ctkDictionary)),
                                "org/mitk/gui/qt/dicom/ADD");

      eventAdmin->publishSignal(this, SIGNAL(SignalAddSeriesToDataManager(ctkDictionary)),
                                "org/mitk/gui/qt/dicom/DELETED");
    }
}