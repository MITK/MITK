/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMITKIGTTrackingToolboxView_h
#define QmitkMITKIGTTrackingToolboxView_h

#include <berryISelectionListener.h>
#include <ctkServiceReference.h>
#include <ctkServiceEvent.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkMITKIGTTrackingToolboxViewControls.h"

//mitk headers
#include <mitkNavigationToolStorage.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>
#include <mitkNavigationDataRecorder.h>
#include <mitkNavigationDataToIGTLMessageFilter.h>
#include <mitkIGTLServer.h>
#include <mitkIGTLMessageProvider.h>

//QT headers
#include <QTimer>

#include "QmitkMITKIGTTrackingToolboxViewWorker.h"

//Forward declaration of MITK classes
namespace mitk
{
  class NeedleProjectionFilter;
}

/*!
  \brief QmitkMITKIGTTrackingToolboxView

   This is the view of the bundle IGT Tracking Toolbox. The IGT Tracking Toolbox can be used to access tracking devices with MITK-IGT. The Tracking Toolbox can be used to log tracking data in     XML or CSV format for measurement purposes. The Tracking Toolbox further allows for visualization of tools with given surfaces in combination with the NaviagtionToolManager.
*/
class QmitkMITKIGTTrackingToolboxView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkMITKIGTTrackingToolboxView();
    ~QmitkMITKIGTTrackingToolboxView() override;

    void CreateQtPartControl(QWidget *parent) override;

    ///
    /// Sets the focus to an internal widget.
    ///
    void SetFocus() override;

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

    /** Freezes the device if it is not frozen / unfreezes the device if it is frozen. */
    void OnFreezeUnfreezeTracking();

    /** @brief Shows or hides the tool projection of the standard tool axis. */
    void OnShowHideToolProjectionClicked();

    /** @brief Shows or hides the standard tool axis. */
    void OnShowHideToolAxisClicked();

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
    void UpdateRenderTrackingTimer();
    void UpdateLoggingTrackingTimer();

    /** @brief Slot for showing the rendering disabled warning label*/
    void OnChangeRenderUpdateRate();

    /** @brief Resets the Tracking Tools: this means all tools are removed. */
    void OnResetTools();

    /** @brief Opens a dialog where a new navigation tool can be created. */
    void OnAddSingleTool();

    /** @brief This slot is called if the user finishes the creation of a new tool. */
    void OnAddSingleToolFinished();

    /** @brief This slot is called if the user cancels the creation of a new tool. */
    void OnAddSingleToolCanceled();

    void OnTimeOut();

    /**
    * \brief This function is called, when anything in the ToolStorage changed, e.g. AddTool or EditTool.
    * ServiceListener is connected in the QmitkMITKIGTTrackingToolboxView.
    */
    void OnToolStorageChanged(const ctkServiceEvent event);

  /* This slot enables selction of tool for projection*/
    void SelectToolProjection(int idx);

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
   void EnableDisableTimerButtons(int enable);
   void OnToggleAdvancedSimpleMode();
   void OnToggleDifferentUpdateRates();

   //slots for worker thread
   void OnAutoDetectToolsFinished(bool success, QString errorMessage);
   void OnConnectFinished(bool success, QString errorMessage);
   void OnStartTrackingFinished(bool success, QString errorMessage);
   void OnStopTrackingFinished(bool success, QString errorMessage);
   void OnDisconnectFinished(bool success, QString errorMessage);

  protected:

    Ui::QmitkMITKIGTTrackingToolboxViewControls* m_Controls;

    bool m_tracking;    ///> bool which is true if tracking is running, false if not
    bool m_connected;   ///> bool that is true when a tracking device is connected
    bool m_logging;     ///> bool which is true if logging is running, false if not
    bool m_ShowHideToolAxis; ///> bool, which will be true, if the tool axis is visible during tracking
    int m_loggedFrames; ///> stores the current number of logged frames if logging is on

    mitk::NavigationToolStorage::Pointer m_toolStorage;  ///>stores the loaded tools
    mitk::DataNode::Pointer m_TrackingVolumeNode;        ///>holds the data node of the tracking volume if volume is visualized
    bool lastTrackingVolumeState;                        ///>temporary holds the state of the tracking volume (activated/not activated) during some methods

    QString m_ToolStorageFilename; ///>stores the filename of the current tool storage
    QString m_AutoSaveFilename; ///>a filename for auto saving tools if no m_ToolStorageFilename was given by the user

    /** @brief Shows a message box with the text given as parameter. */
    void MessageBox(std::string s);

    /** @brief reinits the view globally. */
    void GlobalReinit();

   //members for the filter pipeline
   mitk::TrackingDeviceData m_TrackingDeviceData; ///> stores the tracking device data as long as this is not handled by the tracking device configuration widget
   mitk::NavigationDataObjectVisualizationFilter::Pointer m_ToolVisualizationFilter; ///> holds the tool visualization filter (second filter of the IGT pipeline)
   mitk::NavigationDataRecorder::Pointer m_loggingFilter; ///> holds the logging filter if logging is on (third filter of the IGT pipeline)
   itk::SmartPointer<mitk::NeedleProjectionFilter> m_NeedleProjectionFilter; ///> Contains the needle projection filter which is used for displaying the tool projection and the tool axis during tracking (optional third filter of the IGT pipeline). The filter is updated in the method UpdateRenderTrackingTimer().

   //members for open IGT link server
   mitk::NavigationDataToIGTLMessageFilter::Pointer m_IGTLConversionFilter; ///> Converts the navigation data as open IGT link message and makes this filter available as microservice
   mitk::IGTLServer::Pointer m_IGTLServer;
   mitk::IGTLMessageProvider::Pointer m_IGTLMessageProvider;

   /** @brief This timer updates the IGT pipline and also the logging filter if logging is activated.*/
   QTimer* m_TrackingRenderTimer;
   QTimer* m_TrackingLoggingTimer;
   QTimer* m_TimeoutTimer;

   bool m_SimpleModeEnabled; ///>Stores if simple UI mode is enabled

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
   /**
    * Auto saves the current tool storage to a temporary file. This ist used for persistence.
  */
   void AutoSaveToolStorage();

   /**
   * Shows the projection of the tool along the tool axis for the given tool index
   */
   void ShowToolProjection(int index);
   /**
   * Removes all the tool projections from the data storage
   */
   void RemoveAllToolProjections();


   //members for worker thread
   QThread* m_WorkerThread;
   QmitkMITKIGTTrackingToolboxViewWorker* m_Worker;

  private:
   ctkServiceReference m_DeviceTypeServiceReference;
   mitk::TrackingDeviceTypeCollection* m_DeviceTypeCollection;
   mitk::DataNode::Pointer m_ToolProjectionNode;
};


#endif // _QMITKMITKIGTTRACKINGTOOLBOXVIEW_H_INCLUDED
