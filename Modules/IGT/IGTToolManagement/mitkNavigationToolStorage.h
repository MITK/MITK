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

#ifndef NAVIGATIONTOOLSTORAGE_H_INCLUDED
#define NAVIGATIONTOOLSTORAGE_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include <MitkIGTExports.h>
#include "mitkNavigationTool.h"
#include <mitkDataStorage.h>

// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>

namespace mitk {
  /**Documentation
  * \brief An object of this class represents a collection of navigation tools.
  *        You may add/delete navigation tools or store/load the whole collection
  *        to/from the harddisc by using the class NavigationToolStorageSerializer
  *        and NavigationToolStorageDeserializer.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationToolStorage : public itk::Object
  {
  public:
    mitkClassMacro(NavigationToolStorage,itk::Object);
    /** @brief Constructs a NavigationToolStorage without reference to a DataStorage. The Data Nodes of tools have to be added and removed to a data storage outside this class.
     *         Normaly the other constructor should be used.
     */
    itkNewMacro(Self);
    /** @brief Constructs a NavigationToolStorage with reference to a DataStorage. The Data Nodes of tools are added and removed automatically to this data storage. */
    mitkNewMacro1Param(Self,mitk::DataStorage::Pointer);


    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    * To unregister, call UnregisterMicroservice(). Make sure to pass the id of the Device that this tool is connected to.
    */
    virtual void RegisterAsMicroservice(std::string sourceID);

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    */
    virtual void UnRegisterMicroservice();

    /**
    *\brief Returns the id that this device is registered with. The id will only be valid, if the
    * NavigationDataSource has been registered using RegisterAsMicroservice().
    */
    std::string GetMicroserviceID();

    /**
    *\brief These constants are used in conjunction with Microservices
    */
    static const std::string US_INTERFACE_NAME; // Name of the interface
    static const std::string US_PROPKEY_SOURCE_ID; // ID of the device this ToolStorage is associated with


    /**
     * @brief  Adds a tool to the storage. Be sure that the tool has a unique
     *         identifier which is not already part of this storage.
     * @return Returns true if the tool was added to the storage, false if not
     *         (false can be returned if the identifier already exists in this storage
     *         for example).
     */
    bool AddTool(mitk::NavigationTool::Pointer tool);

    /**
     * @return Returns the tracking tool at the position "number"
     *         in the storage. Returns NULL if there is no
     *         tracking tool at this position.
     */
    mitk::NavigationTool::Pointer GetTool(int number);

    /**
     * @return Returns the tracking tool with the given identifier.
     *         Returns NULL if there is no
     *         tracking tool with this identifier in the storage.
     */
    mitk::NavigationTool::Pointer GetTool(std::string identifier);

    /**
     * @return Returns the tracking tool with the given name.
     *         Returns NULL if there is no
     *         tracking tool with this name in the storage.
     */
    mitk::NavigationTool::Pointer GetToolByName(std::string name);


    /**
     * @brief Deletes a tool from the collection.
     */
    bool DeleteTool(int number);

    /**
     * @brief Deletes all tools from the collection.
     */
    bool DeleteAllTools();

    /**
     * @return Returns the number of tools stored in the storage.
     */
    int GetToolCount();

    /**
     * @return Returns true if the storage is empty, false if not.
     */
    bool isEmpty();

    /**
     * @return Returns the corresponding data storage if one is set to this NavigationToolStorage.
     *         Returns NULL if none is set.
     */
    itkGetMacro(DataStorage,mitk::DataStorage::Pointer);

  protected:
    NavigationToolStorage();
    NavigationToolStorage(mitk::DataStorage::Pointer);
    ~NavigationToolStorage();

    std::vector<mitk::NavigationTool::Pointer> m_ToolCollection;
    mitk::DataStorage::Pointer m_DataStorage;

  private:
    mitk::ServiceRegistration m_ServiceRegistration;

  };
} // namespace mitk
US_DECLARE_SERVICE_INTERFACE(mitk::NavigationToolStorage, "org.mitk.services.NavigationToolStorage")
#endif //NAVIGATIONTOOLSTORAGE
