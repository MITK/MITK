/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkIGTConnectionWidget_H
#define QmitkIGTConnectionWidget_H

#include <QWidget>
#include "MitkIGTUIExports.h"
#include "ui_QmitkIGTConnectionWidgetControls.h"

#include "mitkDataStorage.h"
#include "mitkNavigationToolStorage.h"
#include "mitkTrackingDevice.h"
#include "mitkTrackingDeviceSource.h"


//itk headers

 /** Documentation:
  *   \brief Simple and fast access to a pre-configured TrackingDeviceSource.
  *   
  *   This widget creates a fully configured, connected and started TrackingDeviceSource.
  *   Clicking connect requires to specify a NavigationToolStorage that holds all tools to be used
  *   in the application. Corresponding surfaces are added to the DataStorage that has to be set for
  *   the widget.
  *
  *   \ingroup IGTUI
  */
class MitkIGTUI_EXPORT QmitkIGTConnectionWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkIGTConnectionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkIGTConnectionWidget();
    
    /* @return Returns the preconfigured and connected TrackingDeviceSource ready to use in an IGT pipeline. 
     */ 
    mitk::TrackingDeviceSource::Pointer GetTrackingDeviceSource();
    /*!
    \brief Get the NavigationToolStorage holding all tools with corresponding surface objects
    */
    mitk::NavigationToolStorage::Pointer GetNavigationToolStorage();
    /*!
    \brief set DataStorage that is used to put the navigation tools
    */
    void SetDataStorage(mitk::DataStorage::Pointer dataStorage);
    
    
  signals:

  protected slots:
    /* @brief This method is called when the user changes the selection of the trackingdevice (m_trackingDeviceChooser).
    It then sets the correct widget for the selected tracking device.*/
    void OnTrackingDeviceChanged();
    /*!
    \brief Asks the user to specify a tool file and finally connects the TrackingDeviceSource
    */
    void OnConnect();

  protected:
    
    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    /*!
    \brief Load NavigationToolStorage from given filename and set according member
    \param qFilename file location of the NavigationToolStorage
    \return success of load operation (true if load successful, false otherwise) m_ErrorMessage holds the problem description
    */
    bool LoadToolfile(QString qFilename);

    /*!
    \brief Remove the tool nodes currently associated to the tools hold in the NavigationToolStorage from the DataStorage
    */
    void RemoveToolNodes();
    ///*!
    //\brief Construct TrackingDevice according to the selection in the ComboBox.
    //\return Preconfigured TrackingDevice to be used in a TrackingDeviceSource
    //*/
    //mitk::TrackingDevice::Pointer ConstructTrackingDevice();
    ///* @return Returns a configured NDI 5D tracking device. Unfortunately the NDI 5D tracking device is not yet in the open source part
    // *        so this method only returns NULL at the moment.
    // */
    //virtual mitk::TrackingDevice::Pointer ConfigureNDI5DTrackingDevice();

    ///* @return Returns a configured NDI 6D tracking device. 
    // *         The type (which means Aurora/Polaris) will not be set in the returnvalue. You have to this later.
    // */
    //mitk::TrackingDevice::Pointer ConfigureNDI6DTrackingDevice();

    Ui::QmitkIGTConnectionWidgetControls* m_Controls;

    mitk::DataStorage::Pointer m_DataStorage; ///< data storage to put navigation tools
    mitk::TrackingDevice::Pointer m_TrackingDevice; ///< holds an instance of the currently chosen tracking device which may be of type NDI Polaris, NDI Aurora or Claron MicronTracker
    mitk::TrackingDeviceSource::Pointer m_TrackingDeviceSource; ///< holds the preconfigured source of the IGT pipeline which is provided by this widget for further processing
    mitk::NavigationToolStorage::Pointer m_NavigationToolStorage; ///< holds all navigation tools currently loaded

    std::string m_ErrorMessage; ///< current problem description
};
#endif