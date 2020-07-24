/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomDataEventPublisher_H
#define QmitkDicomDataEventPublisher_H

#include <ctkPluginContext.h>
#include <QObject>

/**
* \brief QmitkDicomDataEventPublisher is a class for publishing ctkEvents.
*/
class QmitkDicomDataEventPublisher : public QObject
{
    Q_OBJECT
    public:

        /**
        * \brief QmitkDicomDataEventPublisher constructor.
        */
        QmitkDicomDataEventPublisher();

        /**
        * \brief QmitkDicomDataEventPublisher destructor.
        */
        ~QmitkDicomDataEventPublisher() override;

        /// @brief sets the event admin from given plugin context
        void PublishSignals(ctkPluginContext* context);

        void AddSeriesToDataManagerEvent(const ctkDictionary& properties);

        void RemoveSeriesFromStorageEvent(const ctkDictionary& properties);

    signals:
        void SignalAddSeriesToDataManager(const ctkDictionary&);

        void SignalRemoveSeriesFromStorage(const ctkDictionary&);
};
#endif // QmitkDicomDataEventPublisher_H
