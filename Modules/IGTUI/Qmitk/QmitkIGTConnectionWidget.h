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
  *   Clicking "Connect" requires to specify a NavigationToolStorage that holds all tools to be used
  *   in the application. Corresponding surfaces are added to the DataStorage that has to be set for
  *   the widget.
  *
  *   Inputs: DataStorage
  *   Outputs: TrackingDeviceSource, NavigationToolStorage
  *   Signals: TrackingDeviceConnected, TrackingDeviceDisconnected
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
    /*!
    \brief signal emitted when TrackingDevice was successfully connected
    */
    void TrackingDeviceConnected();
    /*!
    \brief signal emitted when TrackingDevice was successfully disconnected
    */
    void TrackingDeviceDisconnected();

  protected slots:
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

    Ui::QmitkIGTConnectionWidgetControls* m_Controls;

    mitk::DataStorage::Pointer m_DataStorage; ///< data storage to put navigation tools
    mitk::TrackingDevice::Pointer m_TrackingDevice; ///< tracking device currently connected
    mitk::TrackingDeviceSource::Pointer m_TrackingDeviceSource; ///< holds the preconfigured source of the IGT pipeline which is provided by this widget for further processing
    mitk::NavigationToolStorage::Pointer m_NavigationToolStorage; ///< holds all navigation tools currently loaded

    std::string m_ErrorMessage; ///< current problem description
};
#endif
