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
    
    QmitkMITKIGTTrackingToolboxView(const QmitkMITKIGTTrackingToolboxView& other)
    {
      Q_UNUSED(other)
      throw std::runtime_error("Copy constructor not implemented");
    }
    virtual ~QmitkMITKIGTTrackingToolboxView();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    
    virtual void StdMultiWidgetNotAvailable();

  protected slots:

    /** @brief This slot is called if the user wants to load a new tool file. A new window opens where the user can choose a file. If the chosen file is
               corrupt or not valid the user gets an error message. If the file was loaded successfully the tools are show in the tool status widget. */
    void OnLoadTools();

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
	


  protected:

    Ui::QmitkMITKIGTTrackingToolboxViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    bool m_tracking;    ///> bool which is true if tracking is running, false if not
    bool m_logging;     ///> bool which is true if logging is running, false if not
    int m_loggedFrames; ///> stores the current number of logged frames if logging is on

    mitk::NavigationToolStorage::Pointer m_toolStorage;  ///>stores the loaded tools
    mitk::DataNode::Pointer m_TrackingVolumeNode;        ///>holds the data node of the tracking volume if volume is visualized

    /** @brief Shows a message box with the text given as parameter. */
    void MessageBox(std::string s);

    /** @brief reinits the view globally. */
    void GlobalReinit();

   //members for the filter pipeline
   mitk::TrackingDeviceSource::Pointer m_TrackingDeviceSource; ///> member for the source of the IGT pipeline
   mitk::NavigationDataObjectVisualizationFilter::Pointer m_ToolVisualizationFilter; ///> holds the tool visualization filter (second filter of the IGT pipeline)
   mitk::NavigationDataRecorder::Pointer m_loggingFilter; ///> holds the logging filter if logging is on (third filter of the IGT pipeline)

   /** @brief This timer updates the IGT pipline and also the logging filter if logging is activated.*/
   QTimer* m_TrackingTimer;

   //help methods for enable/disable buttons
   void DisableLoggingButtons();
   void EnableLoggingButtons();
   void DisableOptionsButtons();
   void EnableOptionsButtons();
   void EnableTrackingConfigurationButtons();
   void DisableTrackingConfigurationButtons();

};



#endif // _QMITKMITKIGTTRACKINGTOOLBOXVIEW_H_INCLUDED