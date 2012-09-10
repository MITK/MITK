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

#ifndef _QmitkServiceListWidget_H_INCLUDED
#define _QmitkServiceListWidget_H_INCLUDED

#include "QmitkExports.h"
#include "ui_QmitkServiceListWidgetControls.h"
#include <vector>

//QT headers
#include <QWidget>
#include <QListWidgetItem>

//Microservices
#include "usServiceReference.h"
#include "usModuleContext.h"
#include "usServiceEvent.h"
#include "usServiceInterface.h"



/**
* @brief This widget provides abstraction for the handling of MicroServices . Place one in your Plugin and set it to look for a certain interface.
* One can also specify a filter and / or a property to use for captioning of the services. It also offers functionality to signal
* ServiceEvents and to return the actual classes, so only a minimum of interaction with the MicroserviceInterface is required.
* To get started, just put it in your Plugin or Widget, call the Initialize Method and optionally connect it's signals.
* As QT limits templating possibilities, events only throw ServiceReferences. You can manually dereference them using TranslateServiceReference()
*
* @ingroup QMITK
*/
class QMITK_EXPORT QmitkServiceListWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  private:

    mitk::ModuleContext* m_Context;
    /** \brief a filter to further narrow down the list of results*/
    std::string m_Filter;
    /** \brief The name of the ServiceInterface that this class should list */
    std::string m_Interface;
    /** \brief The name of the ServiceProperty that will be displayed in the list to represent the service */
    std::string m_NamingProperty;

  public:

    static const std::string VIEW_ID;

    QmitkServiceListWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkServiceListWidget();

    /** \brief This method is part of the widget an needs not to be called separately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /** \brief This method is part of the widget an needs not to be called separately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();

    /**
    * \brief Will return true, if a service is currently selected and false otherwise.
    *
    * Call this before requesting service references to avoid invalid ServiceReferences.
    */
    bool GetIsServiceSelected();

    /**
    * \brief Returns the currently selected Service as a ServiceReference.
    *
    * If no Service is selected, the result will probably be a bad pointer. call GetIsServiceSelected()
    * beforehand to avoid this
    */
    mitk::ServiceReference GetSelectedServiceReference();

    /**
    * \brief Use this function to return the currently selected service as a class directly.
    *
    *  Make sure you pass the appropriate type, or else this call will fail.
    *  Usually, you will pass the class itself, not the SmartPointer, but the function returns a pointer. Example:
    *  \verbatim mitk::USDevice::Pointer device = GetSelectedService<mitk::USDevice>(); \endverbatim
    */
    template <class T>
    T* GetSelectedService()
    {
      mitk::ServiceReference ref = GetServiceForListItem( this->m_Controls->m_ServiceList->currentItem() );
      return ( m_Context->GetService<T>(ref) );
    }

    /**
    * \brief  Initializes the Widget with essential parameters.
    *
    * The string filter is an LDAP parsable String, compare mitk::ModuleContext for examples on filtering.
    * Pass class T to tell the widget which class it should filter for - only services of this class will be listed.
    * NamingProperty is a property that will be used to caption the Items in the list. If no filter is supplied, all
    * matching interfaces are shown. If no namingProperty is supplied, the interfaceName will be used to caption Items in the list.
    * For example, this Initialization will filter for all USDevices that are set to active. The USDevice's model will be used to display it in the list:
    * \verbatim
        std::string filter = "(&(" + mitk::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice)(IsActive=true))";
        m_Controls.m_ActiveVideoDevices->Initialize<mitk::USDevice>(mitk::USImageMetadata::PROP_DEV_MODEL ,filter);
    * \endverbatim
    */
    template <class T>
    void Initialize(const std::string& namingProperty = static_cast< std::string >(""),const std::string& filter = static_cast< std::string >(""))
      {
        std::string interfaceName ( us_service_interface_iid<T*>() );
        m_Interface = interfaceName;
        InitPrivate(namingProperty, filter);
      }

    /**
    * \brief Translates a serviceReference to a class of the given type.
    *
    * Use this to translate the signal's parameters. To adhere to the MicroService contract,
    * only ServiceReferences stemming from the same widget should be used as parameters for this method.
    * \verbatim mitk::USDevice::Pointer device = TranslateReference<mitk::USDevice>(myDeviceReference); \endverbatim
    */
    template <class T>
    T* TranslateReference(mitk::ServiceReference reference)
      {
        return dynamic_cast<T*> ( m_Context->GetService<T>(reference) );
      }

    /**
    *\brief This Function listens to ServiceRegistry changes and updates the list of services accordingly.
    *
    * The user of this widget does not need to call this method, it is instead used to recieve events from the module registry.
    */
    void OnServiceEvent(const mitk::ServiceEvent event);

  signals:

    /**
    *\brief Emitted when a new Service matching the filter is being registered.
    *
    * Be careful if you use a filter:
    * If a device does not match the filter when registering, but modifies it's properties later to match the filter,
    * then the first signal you will see this device in will be ServiceModified.
    */
    void ServiceRegistered(mitk::ServiceReference);

    /**
    *\brief Emitted directly before a Service matching the filter is being unregistered.
    */
    void ServiceUnregistering(mitk::ServiceReference);

    /**
    *\brief Emitted when a Service matching the filter changes it's properties, or when a service that formerly not matched the filter
    * changed it's properties and now matches the filter.
    */
    void ServiceModified(mitk::ServiceReference);

    /**
    *\brief Emitted when a Service matching the filter changes it's properties,
    *
    * and the new properties make it fall trough the filter. This effectively means that
    * the widget will not track the service anymore. Usually, the Service should still be useable though
    */
    void ServiceModifiedEndMatch(mitk::ServiceReference);

    /**
    *\brief Emitted if the user selects a Service from the list.
    *
    * If no service is selected, an invalid serviceReference is returned. The user can easily check for this.
    * if (serviceReference) will evaluate to false, if the reference is invalid and true if valid.
    */
    void ServiceSelectionChanged(mitk::ServiceReference);

  public slots:

  protected slots:

    /**
    \brief Called, when the selection in the list of Services changes.
    */
    void OnServiceSelectionChanged();


  protected:

    Ui::QmitkServiceListWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    /**
    * \brief  Internal structure used to link ServiceReferences to their QListWidgetItems
    */
    struct ServiceListLink {
      mitk::ServiceReference service;
      QListWidgetItem* item;
    };

    /**
    * \brief Finishes initialization after Initialize has been called.
    *
    * This function assumes that m_Interface is set correctly (Which Initialize does).
    */
    void InitPrivate(const std::string& namingProperty, const std::string& filter);

    /**
    * \brief Contains a list of currently active services and their entires in the list. This is wiped with every ServiceRegistryEvent.
    */
    std::vector<ServiceListLink> m_ListContent;

    /**
    * \brief Constructs a ListItem from the given service, displays it, and locally stores the service.
    */
    QListWidgetItem* AddServiceToList(mitk::ServiceReference serviceRef);

    /**
    * \brief Removes the given service from the list and cleans up. Returns true if successful, false if service was not found.
    */
    bool RemoveServiceFromList(mitk::ServiceReference serviceRef);

    /**
    * \brief Returns the serviceReference corresponding to the given ListEntry or an invalid one if none was found (will evaluate to false in bool expressions).
    */
    mitk::ServiceReference GetServiceForListItem(QListWidgetItem* item);

    /**
    * \brief Returns a list of ServiceReferences matching the filter criteria by querying the service registry.
    */
    std::list<mitk::ServiceReference> GetAllRegisteredServices();



};

#endif // _QmitkServiceListWidget_H_INCLUDED
