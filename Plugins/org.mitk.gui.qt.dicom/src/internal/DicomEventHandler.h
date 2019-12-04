/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef DicomEventHandler_h
#define DicomEventHandler_h

#include <QObject>
#include <service/event/ctkEvent.h>

/**
* \brief DicomEventHandler is a class for handling dicom events between dicom plugin and datamanager.
*/
class DicomEventHandler : public QObject
{
    Q_OBJECT
public:

    /**
    * \brief DicomEventHandler constructor.
    */
    DicomEventHandler();

    /**
    * \brief DicomEventHandler destructor.
    */
    ~DicomEventHandler() override;

    /**
    * \brief Subscribes slots in this class.
    *
    * This function will subscribe OnSignalAddSeriesToDataManager and OnSignalRemoveSeriesFromStorage.
    * Doing this allows to react on ctkEvents with EVENT_TOPIC "org/mitk/gui/qt/dicom/ADD" and "org/mitk/gui/qt/dicom/DELETED".
    */
    void SubscribeSlots();

    public slots:

        /**
        * \brief Called when ctkEvent with EVENT_TOPIC "org/mitk/gui/qt/dicom/ADD" is thrown.
        *
        * If this slot catches an event it will add the dicom file provided by event properties.
        */
        void OnSignalAddSeriesToDataManager(const ctkEvent& ctkEvent);

        /**
        * \brief Called when ctkEvent with EVENT_TOPIC "org/mitk/gui/qt/dicom/DELETED" is thrown.
        *
        * \note Not yet implemented.
        */
        void OnSignalRemoveSeriesFromStorage(const ctkEvent& ctkEvent);
};
#endif // QmitkDicomEventHandlerBuilder_h
