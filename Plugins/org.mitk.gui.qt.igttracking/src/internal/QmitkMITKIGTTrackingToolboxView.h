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

#ifndef QmitkMITKIGTTrackingToolboxView_h
#define QmitkMITKIGTTrackingToolboxView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkMITKIGTTrackingToolboxViewControls.h"

//mitk headers
#include <mitkNavigationToolStorage.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>
#include <mitkNavigationDataRecorder.h>

//QT headers
#include <QTimer>

class QmitkMITKIGTTrackingToolboxViewWorker;

/*!
  \brief QmitkMITKIGTTrackingToolboxView

   This is the view of the bundle IGT Tracking Toolbox. The IGT Tracking Toolbox can be used to access tracking devices with MITK-IGT. The Tracking Toolbox can be used to log tracking data in     XML or CSV format for measurement purposes. The Tracking Toolbox further allows for visualization of tools with given surfaces in combination with the NaviagtionToolManager.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkMITKIGTTrackingToolboxView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkMITKIGTTrackingToolboxView();
    virtual ~QmitkMITKIGTTrackingToolboxView();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);

    virtual void StdMultiWidgetNotAvailable();

  protected slots:

    /** @brief changes name of the filename when switching fileextension by radio button */
    void OnToggleFileExtension();
    /** @brief This slot is called if the user wants to load a new tool file. A new window opens where the user can choose a file. If the chosen file is
               corrupt or not valid the user gets an error message. If the file was loaded successfully the tools are show in the tool status widget. */
    void OnLoadTools();

    /** Starts tracking if tracking is stopped / stops tracking if tracking is started. */
    void OnStartStopTracking();

    /** Connects the device if it is disconnected / disconnects the device if it is connected. */
    void OnConnectDisconnect();

    /** @brief This slot connects to the device. In status "connected" configuration of the device is disabled. */
    void OnConnect();

    /** @brief This slot disconnects from the device. */
    void OnDisconnect();

    /** @brief This slot tries to start tracking with the current device. If start tracking fails the user gets an error message and tracking stays off.*/
    void OnStartTracking();

    /** @brief This slot stops tracking. If tracking is not strated it does nothing.*/
    void OnStopTracking();

    /** @brief This slot is called if the user want's to choose a file name for logging. A new windows to navigate through the file system and choose
               a file opens.*/
    void OnChooseFileClicked();

    /** @brief This slot starts logging. Logging is only possible if a device is tracking. If not the logging mechanism start when the start tracking
               is called.*/
    void StartLogging();

    /** @brief This slot stops logging. If logging is not running it does nothing.*/
    void StopLogging();

    /** @brief This slot enables / disables UI elements depending on the tracking device after a device is changed.*/
    void OnTrackingDeviceChanged();

    /** @brief This slot selects the Tracking Volume appropriate for a given model */
    void OnTrackingVolumeChanged(QString qstr);

    /** @brief Shows or hides the tracking volume according to the checkboxe's state */
    void OnShowTrackingVolumeChanged();

    /** @brief This slot auto detects tools of a NDI Aurora tracking device. If tools where found they will be stored internally as a tool storage.
        The user is also asked if he wants to save this tool storage to load it later. Only call it if a Aurora device was configured because other
        devices don't support auto detection.*/
    void OnAutoDetectTools();

    /** @brief Slot for tracking timer. The timer updates the IGT pipline and also the logging filter if logging is activated.*/
    void UpdateTrackingTimer();

    /** @brief Resets the Tracking Tools: this means all tools are removed. */
    void OnResetTools();

    /** @brief Opens a dialog where a new navigation tool can be created. */
    void OnAddSingleTool();

    /** @brief This slot is called if the user finishes the creation of a new tool. */
    void OnAddSingleToolFinished();

    /** @brief This slot is called if the user cancels the creation of a new tool. */
    void OnAddSingleToolCanceled();

    void OnTimeOut();

  protected slots:

   //help slots for enable/disable buttons
   void DisableLoggingButtons();
   void EnableLoggingButtons();
   void DisableOptionsButtons();
   void EnableOptionsButtons();
   void EnableTrackingConfigurationButtons();
   void DisableTrackingConfigurationButtons();
   void EnableTrackingControls();
   void DisableTrackingControls();

   //slots for worker thread
   void OnAutoDetectToolsFinished(bool success, QString errorMessage);
   void OnConnectFinished(bool success, QString errorMessage);
   void OnStartTrackingFinished(bool success, QString errorMessage);
   void OnStopTrackingFinished(bool success, QString errorMessage);
   void OnDisconnectFinished(bool success, QString errorMessage);

  protected:

    Ui::QmitkMITKIGTTrackingToolboxViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    bool m_tracking;    ///> bool which is true if tracking is running, false if not
    bool m_connected;   ///> bool that is true when a tracking device is connected
    bool m_logging;     ///> bool which is true if logging is running, false if not
    int m_loggedFrames; ///> stores the current number of logged frames if logging is on

    mitk::NavigationToolStorage::Pointer m_toolStorage;  ///>stores the loaded tools
    mitk::DataNode::Pointer m_TrackingVolumeNode;        ///>holds the data node of the tracking volume if volume is visualized
    bool lastTrackingVolumeState;                        ///>temporary holds the state of the tracking volume (activated/not activated) during some methods

    QString m_ToolStorageFilename; ///>stores the filename of the current tool storage

    /** @brief Shows a message box with the text given as parameter. */
    void MessageBox(std::string s);

    /** @brief reinits the view globally. */
    void GlobalReinit();

   //members for the filter pipeline
   mitk::TrackingDeviceSource::Pointer m_TrackingDeviceSource; ///> member for the source of the IGT pipeline
   mitk::TrackingDeviceData m_TrackingDeviceData; ///> stores the tracking device data as long as this is not handled by the tracking device configuration widget
   mitk::NavigationDataObjectVisualizationFilter::Pointer m_ToolVisualizationFilter; ///> holds the tool visualization filter (second filter of the IGT pipeline)
   mitk::NavigationDataRecorder::Pointer m_loggingFilter; ///> holds the logging filter if logging is on (third filter of the IGT pipeline)

   /** @brief This timer updates the IGT pipline and also the logging filter if logging is activated.*/
   QTimer* m_TrackingTimer;
   QTimer* m_TimeoutTimer;

   /** Replaces the current navigation tool storage which is stored in m_toolStorage.
    *  Basically handles the microservice stuff: unregisteres the old storage, then
    *  replaces the storage and registers the new one.
    */
   void ReplaceCurrentToolStorage(mitk::NavigationToolStorage::Pointer newStorage, std::string newStorageName);

   /**
    * \brief Stores the properties of some QWidgets (and the tool storage file name) to QSettings.
    */
   void StoreUISettings();

   /**
    * \brief Loads the properties of some QWidgets (and the tool storage file name) from QSettings.
    */
   void LoadUISettings();
   /**
    * Help method for updating the tool label
    */
   void UpdateToolStorageLabel(QString pathOfLoadedStorage);

   //members for worker thread
   QThread* m_WorkerThread;
   QmitkMITKIGTTrackingToolboxViewWorker* m_Worker;
};


/**
 * Worker thread class for this view.
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

  void SetWorkerMethod(WorkerMethod w);
  void SetTrackingDevice(mitk::TrackingDevice::Pointer t);
  void SetDataStorage(mitk::DataStorage::Pointer d);
  void SetInverseMode(bool mode);
  void SetTrackingDeviceData(mitk::TrackingDeviceData d);
  void SetNavigationToolStorage(mitk::NavigationToolStorage::Pointer n);

  itkGetMacro(NavigationToolStorage,mitk::NavigationToolStorage::Pointer);

  itkGetMacro(TrackingDeviceSource,mitk::TrackingDeviceSource::Pointer);
  itkGetMacro(TrackingDeviceData,mitk::TrackingDeviceData);
  itkGetMacro(ToolVisualizationFilter,mitk::NavigationDataObjectVisualizationFilter::Pointer);

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
    std::map<mitk::DataNode::Pointer,mitk::Color> m_OriginalColors;

    //internal methods
    void AutoDetectTools();
    void ConnectDevice();
    void StartTracking();
    void StopTracking();
    void DisconnectDevice();
};



#endif // _QMITKMITKIGTTRACKINGTOOLBOXVIEW_H_INCLUDED
