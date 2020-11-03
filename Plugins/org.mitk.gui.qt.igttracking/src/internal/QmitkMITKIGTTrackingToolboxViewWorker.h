/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMITKIGTTrackingToolboxViewWorker_h
#define QmitkMITKIGTTrackingToolboxViewWorker_h

#include <berryISelectionListener.h>

//mitk headers
#include <mitkNavigationToolStorage.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>

//QT headers
#include <QTimer>

/**
 * Worker thread class for QmitkMITKIGTTrackingToolboxView view.
 */
class QmitkMITKIGTTrackingToolboxViewWorker : public QObject
{
  Q_OBJECT

public:
  enum WorkerMethod{
    eAutoDetectTools = 0,
    eConnectDevice = 1,
    eStartTracking = 2,
    eStopTracking = 3,
    eDisconnectDevice = 4
  };

  QmitkMITKIGTTrackingToolboxViewWorker();
  ~QmitkMITKIGTTrackingToolboxViewWorker() override;

  void SetWorkerMethod(WorkerMethod w);
  void SetTrackingDevice(mitk::TrackingDevice::Pointer t);
  void SetDataStorage(mitk::DataStorage::Pointer d);
  void SetInverseMode(bool mode);
  void SetTrackingDeviceData(mitk::TrackingDeviceData d);
  void SetNavigationToolStorage(mitk::NavigationToolStorage::Pointer n);

  itkGetMacro(NavigationToolStorage, mitk::NavigationToolStorage::Pointer);

  mitk::TrackingDeviceSource::Pointer GetTrackingDeviceSource();
  itkGetMacro(TrackingDeviceData, mitk::TrackingDeviceData);
  itkGetMacro(ToolVisualizationFilter, mitk::NavigationDataObjectVisualizationFilter::Pointer);

  public slots:
  void ThreadFunc();

signals:
  void AutoDetectToolsFinished(bool success, QString errorMessage);
  void ConnectDeviceFinished(bool success, QString errorMessage);
  void StartTrackingFinished(bool success, QString errorMessage);
  void StopTrackingFinished(bool success, QString errorMessage);
  void DisconnectDeviceFinished(bool success, QString errorMessage);

protected:

  mitk::TrackingDevice::Pointer m_TrackingDevice;
  WorkerMethod m_WorkerMethod;
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::NavigationToolStorage::Pointer m_NavigationToolStorage;

  //members for the filter pipeline which is created in the worker thread during ConnectDevice()
  mitk::TrackingDeviceSource::Pointer m_TrackingDeviceSource; ///> member for the source of the IGT pipeline
  mitk::TrackingDeviceData m_TrackingDeviceData; ///> stores the tracking device data as long as this is not handled by the tracking device configuration widget
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_ToolVisualizationFilter; ///> holds the tool visualization filter (second filter of the IGT pipeline)

  //members some internal flags
  bool m_InverseMode;     //flag that is true when the inverse mode is enabled

  //stores the original colors of the tracking tools
  std::map<mitk::DataNode::Pointer, mitk::Color> m_OriginalColors;

  //internal methods
  void AutoDetectTools();
  void ConnectDevice();
  void StartTracking();
  void StopTracking();
  void DisconnectDevice();
};

#endif // _QMITKMITKIGTTRACKINGTOOLBOXVIEWWorker_H_INCLUDED
