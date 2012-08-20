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

#ifndef QmitkDicomDataEventPublisher_H
#define QmitkDicomDataEventPublisher_H

#include <ctkPluginContext.h>
#include <QObject>

class QmitkDicomDataEventPublisher : public QObject
{
    Q_OBJECT
    public:

        QmitkDicomDataEventPublisher();

        virtual ~QmitkDicomDataEventPublisher();

        /// @brief sets the event admin from given plugin context
        void PublishSignals(ctkPluginContext* context);

        void AddSeriesToDataManagerEvent(const ctkDictionary& properties);

        void RemoveSeriesFromStorageEvent(const ctkDictionary& properties);

    signals:
        void SignalAddSeriesToDataManager(const ctkDictionary&);

        void SignalRemoveSeriesFromStorage(const ctkDictionary&);
};
#endif // QmitkDicomDataEventPublisher_H