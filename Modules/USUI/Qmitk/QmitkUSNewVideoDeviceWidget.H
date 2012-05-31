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

#ifndef _QmitkUSNewVideoDeviceWidget_H_INCLUDED
#define _QmitkUSNewVideoDeviceWidget_H_INCLUDED

#include "mitkUSUIExports.h"
#include "ui_QmitkUSNewVideoDeviceWidgetControls.h"
#include "mitkUSDeviceService.h"

//QT headers
#include <QWidget>
#include <QListWidgetItem>

//mitk header

/**
* @brief TODO
*
* @ingroup USUI
*/
class MitkUSUI_EXPORT QmitkUSNewVideoDeviceWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkUSNewVideoDeviceWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkUSNewVideoDeviceWidget();

    void Initialize(mitk::USDeviceService::Pointer deviceService);

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();
  
  signals:

    /*
    \brief Sent, when the user clicks "Activate Device"
    */
    void USDeviceActivated();

  public slots:
    /*
    \brief Should be called from the Implementing bundle whenever changes to the DeviceService happen
    */
    //void OnDeviceServiceUpdated();

  protected slots:
  
    /*
    \brief Called, when the Button Activate Device was clicked
    */
    void OnClickedActivateDevice();

    /*
    \brief Called, when the Button Disconnect Device was clicked
    */
    void OnClickedDisconnectDevice();   



  protected:

    Ui::QmitkUSNewVideoDeviceWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    /*
    \brief Constrcuts a ListItem from the given devie for display in the list of active devices
    */
    QListWidgetItem* ConstructItemFromDevice(mitk::USDevice::Pointer device);
 

  private:

    mitk::USDeviceService::Pointer m_DeviceService;

};

#endif // _QmitkUSNewVideoDeviceWidget_H_INCLUDED
