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

/**
* @brief This widget provides abstraction for MicroServices. Place one in your Plugin and set it to a certain interface.
* One can also specify a filter and / or a property to use for captioning of the services. It also offers functionality to be
* informed of ServiceEvents and to return the sctual classes, so only a minimum of interaction with the MicroserviceInterface is required.
*
* @ingroup QMITK
*/
class QMITK_EXPORT QmitkServiceListWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkServiceListWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkServiceListWidget();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();

    /*
    * \brief  Initializes the connection to the registry. The string filter is an LDAP parsable String, compare mitk::ModuleContext for examples on filtering.
    * interfaceName is the name of the interface that is defined in the classes header file and that is used register it it with the MicroServices. NamingProperty
    * is a property that will be used to caption the Items in the list. If no filter is supplied, all matching interfaces are shown. If no namingProperty is supplied,
    * the interfaceName will be used to caption Items in the list.
    */
    void Initialize(std::string interfaceName, std::string namingProperty, std::string filter);

    /*
    * \brief Use this function to returns the currently selected service as a class directly.
    *  Make sure you pass the appropriate type, or else this call will fail.
    */
    template <class T>
    T* GetSelectedService2();

    /*
    * \brief Returns the currently selected Service as a ServiceReference.
    */
    mitk::ServiceReference GetSelectedService();

    /*
    *\brief This Function listens to ServiceRegistry changes and updates the
    *       list of services accordingly. The user of this widget does not need to call this method,
    *       it is instead used to recieve events from the module registry.
    */
    void OnServiceEvent(const mitk::ServiceEvent event);

    mitk::ModuleContext* provideContext();



  signals:

    /*
    *\brief Emitted when a new Service mathing the filter is being registered.
    */
    void ServiceRegistered(mitk::ServiceReference);

    /*
    *\brief Emitted directly before a Service matching the filter is being unregistered.
    */
    void ServiceUnregistering(mitk::ServiceReference);

    /*
    *\brief Emitted when a Service matching the filter changes it's properties.
    */
    void ServiceModified(mitk::ServiceReference);

    /*
    *\brief Emitted the user selects a Service from the list
    */
    void ServiceSelected(mitk::ServiceReference);

  public slots:

  protected slots:

    /*
    \brief Called, when the selection in the list of Services changes
    */
    void OnServiceSelectionChanged();


  protected:

    Ui::QmitkServiceListWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    /*
    * \brief  Internal structure used to link ServiceReferences to their QListWidgetItems
    */
    struct ServiceListLink {
      mitk::ServiceReference service;
      QListWidgetItem* item;
    };

    /*
    * \brief Contains a list of currently active services and their entires in the list. This is wiped with every ServiceRegistryEvent.
    */
    std::vector<ServiceListLink> m_ListContent;

    /*
    * \brief Constructs a ListItem from the given service, displays it, and locally stores the service.
    */
    QListWidgetItem* AddServiceToList(mitk::ServiceReference serviceRef);

    /*
    * \brief Removes the given service from the list and cleans up. Returns true if successful, false if service was not found.
    */
    bool RemoveServiceFromList(mitk::ServiceReference serviceRef);

    /*
    * \brief Returns the serviceReference corresponding to the given ListEntry or null if none was found (which really shouldn't happen).
    */
    mitk::ServiceReference GetServiceForListItem(QListWidgetItem* item);

    /*
    * \brief Returns a list of ServiceReferences matching the filter criteria by querying the service registry.
    */
    std::list<mitk::ServiceReference> GetAllRegisteredServices();


  private:

    mitk::ModuleContext* m_Context;
    /** \brief a filter to further narrow down the list of results **/
    std::string m_Filter;
    /** \brief The name of the ServiceInterface that this class should list **/
    std::string m_Interface;
    /** \brief The name of the ServiceProperty that will be displayed in the List to represent the service **/
    std::string m_NamingProperty;
};

#endif // _QmitkServiceListWidget_H_INCLUDED
