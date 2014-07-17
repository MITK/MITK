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

#ifndef _QmitkUSDeviceManagerWidget_H_INCLUDED
#define _QmitkUSDeviceManagerWidget_H_INCLUDED

#include "MitkUSUIExports.h"
#include "ui_QmitkUSDeviceManagerWidgetControls.h"
#include "mitkUSDevice.h"
#include <vector>

//QT headers
#include <QWidget>
#include <QListWidgetItem>



/**
* @brief This Widget is used to manage available Ultrasound Devices.
*
* It allows activation, deactivation and disconnection of connected devices.
*
* @ingroup USUI
*/
class MitkUSUI_EXPORT QmitkUSDeviceManagerWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkUSDeviceManagerWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkUSDeviceManagerWidget();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();
    /* @brief Disconnects all devices immediately. */
    virtual void DisconnectAllDevices();

  signals:
    void NewDeviceButtonClicked();

    /* This signal is emitted if a device is activated. */
    void DeviceActivated();

  public slots:

  protected slots:

    /*
    \brief Called, when the button "Activate Device" was clicked.
    */
    void OnClickedActivateDevice();

    /*
    \brief Called, when the button "Disconnect Device" was clicked.
    */
    void OnClickedDisconnectDevice();

    void OnClickedRemoveDevice();
    void OnClickedNewDevice();

    /*
    \brief Called, when the selection in the devicelist changes.
    */
    void OnDeviceSelectionChanged(us::ServiceReferenceU reference);


  protected:

    Ui::QmitkUSDeviceManagerWidgetControls* m_Controls; ///< member holding the UI elements of this widget

  private:


};

#endif // _QmitkUSDeviceManagerWidget_H_INCLUDED
