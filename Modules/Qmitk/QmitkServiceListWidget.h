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
* @brief TODO
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
    */
    void Initialize(std::string interfaceName, std::string filter);

    /*
    * \brief TODO
    */
    template <class T>
    T GetSelectedService();

    /*
    * \brief TODO
    */
    mitk::ServiceReference* GetSelectedService();



    /*
    *\brief This Function listens to ServiceRegistry changes and updates the
    *       list of devices accordingly. The User of this Widget does not need to use this method,
    *       it is instead used to recieve events from the module registry.
    */
    void OnServiceEvent(const mitk::ServiceEvent event);



  signals:

    /*
    *\brief Emitted when a new service mathing the filter connects.
    */
    void ServiceConnected(mitk::ServiceReference*);

    /*
    *\brief Emitted directly before a service matching the filter disconnects.
    */
    void ServiceDisconnected(mitk::ServiceReference*);

    /*
    *\brief Emitted when a service mathing the filter changes it's properties.
    */
    void ServiceChanged(mitk::ServiceReference*);

    /*
    *\brief Emitted the user selects a Service from the list
    */
    void ServiceSelected(mitk::ServiceReference*);

  public slots:

  protected slots:

    /*
    \brief Called, when the selection in the servicelist changes
    */
    void OnServiceSelectionChanged();


  protected:

    Ui::QmitkServiceListWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    /*
    * \brief  Internal Structure used to link services to their QListWidgetItems
    */
    struct ServiceListLink {
      mitk::ServiceReference* service;
      QListWidgetItem* item;
    };

    /*
    * \brief  Contains a list of currently active servicesand their entires in the list. This is wiped with every ServiceRegistryEvent.
    */
    std::vector<ServiceListLink> m_ListContent;

    /*
    * \brief Constructs a ListItem from the given service for display in the list of services.
    */
    QListWidgetItem* ConstructItemFromService(mitk::ServiceReference* serviceRef);

    /*
    * \brief Returns the serviceReference corresponding to the given ListEntry or null if none was found (which really shouldn't happen).
    */
    mitk::ServiceReference* GetServiceForListItem(QListWidgetItem* item);

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
};

#endif // _QmitkServiceListWidget_H_INCLUDED
