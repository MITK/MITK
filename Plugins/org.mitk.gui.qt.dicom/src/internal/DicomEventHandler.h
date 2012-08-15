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

#ifndef DicomEventHandler_h
#define DicomEventHandler_h

#include <QObject>
#include <service/event/ctkEvent.h>

class DicomEventHandler : public QObject
{
    Q_OBJECT
public:

    DicomEventHandler();

    virtual ~DicomEventHandler();

    void SubscribeSlots();

    public slots:

        void OnSignalAddSeriesToDataManager(const ctkEvent& ctkEvent);

        void OnSignalRemoveSeriesFromStorage(const ctkEvent& ctkEvent);
};
#endif // QmitkDicomEventHandlerBuilder_h