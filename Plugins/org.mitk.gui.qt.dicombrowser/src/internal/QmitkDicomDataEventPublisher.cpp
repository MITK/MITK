/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
