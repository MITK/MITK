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

#ifndef _QmitkUSDeviceListWidget_H_INCLUDED
#define _QmitkUSDeviceListWidget_H_INCLUDED

#include "MitkUSUIExports.h"
#include "ui_QmitkUSDeviceListWidgetControls.h"
#include "mitkUSDevice.h"
#include <vector>

//QT headers
#include <QWidget>
#include <QListWidgetItem>

//mitk header

//Microservices
#include "usServiceReference.h"
#include "usModuleContext.h"
#include "usServiceEvent.h"
#include "usServiceTrackerCustomizer.h"

/**
* @brief TODO
*
* @ingroup USUI
*/
class MitkUSUI_EXPORT QmitkUSDeviceListWidget :public QWidget //,  public mitk::ServiceTrackerCustomizer<> // this extension is necessary if one wants to use ServiceTracking instead of filtering
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkUSDeviceListWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkUSDeviceListWidget();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();
  
    /*
    * \brief  Initializes the connection to the registry. The string filter is an LDAP parsable String, compare mitk::ModuleContext for examples on filtering.
    */
    void Initialize(std::string filter);

    /*
    * \brief Returns the currently selected device, or null if none is selected.
    */
    mitk::USDevice::Pointer GetSelectedDevice();

    /*
    *\brief This Function listens to ServiceRegistry changes and updates the
    *       list of devices accordingly.
    */
    void OnServiceEvent(const mitk::ServiceEvent event);

    

  signals:

    /*
    *\brief Emitted when a new device mathing the filter connects
    */
    void DeviceConnected(mitk::USDevice::Pointer);
    /*
    *\brief Emitted directly before device matching the filter disconnects
    */
    void DeviceDisconnected(mitk::USDevice::Pointer);
    /*
    *\brief Emitted when a new device mathing the filter changes it's state. This does of now only compromise changes to activity.
    */
    void DeviceChanged(mitk::USDevice::Pointer);
    
    /*
    *\brief Emitted the user selects a device from the list
    */
    void DeviceSelected(mitk::USDevice::Pointer);

    

  public slots:

  protected slots:

    /*
    \brief Called, when the selection in the devicelist changes
    */
    void OnDeviceSelectionChanged();   


  protected:

    Ui::QmitkUSDeviceListWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    /*
    * \brief  Internal Structure used to link devices to their QListWidget Items
    */
    struct DeviceListLink {   
      mitk::USDevice::Pointer device;
      QListWidgetItem* item;
    };

    /*
    * \brief  Contains a list of currently active devices and their entires in the list. This is wiped with every ServiceRegistryEvent.
    */
    std::vector<DeviceListLink> m_ListContent;

    /*
    * \brief Constructs a ListItem from the given device for display in the list of active devices.
    */
    QListWidgetItem* ConstructItemFromDevice(mitk::USDevice::Pointer device);

    /*
    * \brief Returns the device corresponding to the given ListEntry or null if none was found (which really shouldnt happen).
    */
    mitk::USDevice::Pointer GetDeviceForListItem(QListWidgetItem* item);

    //mitk::ServiceTracker<mitk::USDevice, mitk::USDevice::Pointer> ConstructServiceTracker();
 
    /*
    * \brief Returns a List of US Devices that are currently connected by querying the service registry.
    */
    std::vector<mitk::USDevice::Pointer> GetAllRegisteredDevices();


    

  private:

    mitk::ModuleContext* m_MitkUSContext;
    std::string m_Filter;

    


    
};

#endif // _QmitkUSDeviceListWidget_H_INCLUDED
