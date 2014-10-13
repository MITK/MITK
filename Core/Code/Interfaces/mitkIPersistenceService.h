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
#ifndef mitkIPersistenceService_h
#define mitkIPersistenceService_h

// mitk
#include "mitkPropertyListReplacedObserver.h"
#include "usServiceReference.h"
#include "usModuleContext.h"
#include "usGetModuleContext.h"
//for microservices
#include <mitkServiceInterface.h>
#include "mitkDataStorage.h"

namespace mitk
{
    /**
    * The central service for the persistence module
    * Basic idea is to create PropertyLists with a unique id using AddPropertyList(). A consumer
    * of this interface can write arbitrary information into this propertylist
    * Calling Save() and Load() will cause the Service to save and load the current set of propertlists from
    * a file in the user directory.
    * Using SetAutoLoadAndSave(true) will cause the service to load/save the property lists at application
    * start/stop.
    * Moreover, depending on the backend type, the service is connected to the SceneSerialization module, i.e.
    * the user will be asked whether to save/load the propertlists in/from the current ".mitk" file that is selected
    * by the user.
    */
    class MITK_CORE_EXPORT IPersistenceService
    {
    public:
        /**
        * If PropertyList with the given id exists, returns it. Otherwise creates a new one and returns it.
        * If id is empty a UUID will be created and set on the variable
        * If existed was passed, it is true if the PropertyList with that id existed, false otherwise
        * \return a valid PropertyList with a StringProperty "Id" containing the passed id
        */
        virtual mitk::PropertyList::Pointer GetPropertyList( std::string& id, bool* existed=0 ) = 0;
        /**
        * removes the PropertyList with the given id
        * \return true if PropertyList existed and could be removed, false otherwise
        */
        virtual bool RemovePropertyList( std::string& id ) = 0;
        /**
        * Get the default name of the PersistenceFile (the one that is loaded at startup)
        */
        virtual std::string GetDefaultPersistenceFile() = 0;
        /**
        * \return The name of the Bool Property that specifies whether a DataNode is a Node carrying Persistence PropertyLists
        */
        virtual std::string GetPersistenceNodePropertyName() = 0;
        /**
        * Creates a vector of DataNodes that contain all PropertyLists. Additionally, the DataNodes
        * will have the property name set to the PropertyList's id and a BoolProperty equal to GetPersistenceNodePropertyName() set to true. If ds is set the returned DataNodes will also be added to that DS.
        * \return vector of DataNodes with the described attributes
        */
        virtual DataStorage::SetOfObjects::Pointer GetDataNodes(DataStorage* ds=0) = 0;
        /**
        * Searches storage for persistent DataNodes, extracts and inserts the appended property lists to this service
        * \return true if at least one node was found from which a PropertyList could be restored
        */
        virtual bool RestorePropertyListsFromPersistentDataNodes(const DataStorage* storage) = 0;
        /**
        * Save the current PropertyLists to fileName. If fileName is empty, a special file in the users home directory will be used.
        * if appendchanges is true, the file will not replaced but first loaded, then overwritten and then replaced
        * \return false if an error occured (cannot write to file), true otherwise
        */
        virtual bool Save(const std::string& fileName="", bool appendChanges=false) = 0;
        /**
        * Load PropertyLists from fileName. If fileName is empty, a special file in the users home directory will be used.
        * If enforeReload is false, the service will take care of modified time flags, i.e. it will not load a file
        * that was loaded before and did not change in the meantime or that was modified by the service itself
        * *ATTENTION*: If there are PropertyLists with the same id contained in the file, existing PropertyLists will be overwritten!
        * \see AddPropertyListReplacedObserver()
        * \return false if an error occured (cannot load from file), true otherwise
        */
        virtual bool Load(const std::string& fileName="", bool enforeReload=true) = 0;
        /**
        * Using SetAutoLoadAndSave(true) will cause the service to load/save the property lists at application
        * start/stop.
        */
        virtual void SetAutoLoadAndSave(bool autoLoadAndSave) = 0;
        /**
        * \return whether AutoLoading is activated or not
        */
        virtual bool GetAutoLoadAndSave() = 0;
        /**
        * adds a observer which is informed if a propertyList gets replaced during a Load() procedure
        */
        virtual void AddPropertyListReplacedObserver( PropertyListReplacedObserver* observer ) = 0;
        /**
        * removes a specific observer
        */
        virtual void RemovePropertyListReplacedObserver( PropertyListReplacedObserver* observer ) = 0;
        /**
        * nothing to do here
        */
        virtual ~IPersistenceService();
    };
}

// MACROS FOR AUTOMATIC SAVE FUNCTION
#define PERSISTENCE_GET_MODULE_CONTEXT_FUNCTION\
    us::GetModuleContext()

#define PERSISTENCE_GET_SERVICE_MACRO\
    mitk::IPersistenceService* persistenceService = 0;\
    us::ModuleContext* context = PERSISTENCE_GET_MODULE_CONTEXT_FUNCTION;\
    if( context )\
    {\
        us::ServiceReference<mitk::IPersistenceService> persistenceServiceRef = context->GetServiceReference<mitk::IPersistenceService>();\
        if( persistenceServiceRef )\
        {\
            persistenceService\
                = dynamic_cast<mitk::IPersistenceService*> ( context->GetService<mitk::IPersistenceService>(persistenceServiceRef) );\
        }\
    }

#define PERSISTENCE_GET_SERVICE_METHOD_MACRO\
    mitk::IPersistenceService* GetPeristenceService() const\
    {\
        PERSISTENCE_GET_SERVICE_MACRO\
        return persistenceService;\
    }

#define PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
public:\
    bool Save(const std::string& fileName="", bool appendChanges=false)\
    {\
        mitk::IPersistenceService* persistenceService = this->GetPeristenceService();\
        bool noError = persistenceService != 0;\
        if( noError )\
            this->ToPropertyList();\
        if(noError)\
            noError = persistenceService->Save(fileName, appendChanges);\
        return noError;\
    }\
    bool Load(const std::string& fileName="", bool enforeReload=true)\
    {\
        mitk::IPersistenceService* persistenceService = this->GetPeristenceService();\
        bool noError = persistenceService != 0 && persistenceService->Load(fileName, enforeReload);\
        if( noError )\
        {\
            this->FromPropertyList();\
        }\
        return noError;\
    }\
    void ToPropertyList()\
    {\
        mitk::IPersistenceService* persistenceService = this->GetPeristenceService();\
        this->InitializePropertyListReplacedObserver(persistenceService);\
        if( !persistenceService )\
            return;\
        mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(ID_MEMBER_NAME);

#define PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
    }\
    void FromPropertyList()\
    {\
        mitk::IPersistenceService* persistenceService = this->GetPeristenceService();\
        this->InitializePropertyListReplacedObserver(persistenceService);\
        if( !persistenceService )\
            return;\
        mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(ID_MEMBER_NAME);

#define PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)\
}\
std::string GetId() const\
{\
    return ID_MEMBER_NAME;\
}\
private:\
    PERSISTENCE_GET_SERVICE_METHOD_MACRO\
    struct MyPropertyListReplacedObserver: public mitk::PropertyListReplacedObserver\
    {\
        MyPropertyListReplacedObserver()\
        : m_##THE_CLASS_NAME(0), m_PersistenceService(0)\
        {\
        }\
        ~MyPropertyListReplacedObserver()\
        {\
            if( m_PersistenceService )\
                m_PersistenceService->RemovePropertyListReplacedObserver(this);\
        }\
        void AfterPropertyListReplaced( const std::string& id, mitk::PropertyList* propertyList )\
        {\
            if( m_##THE_CLASS_NAME && m_##THE_CLASS_NAME->GetId() == id )\
                m_##THE_CLASS_NAME->FromPropertyList();\
        }\
        void Initialize( THE_CLASS_NAME * _##THE_CLASS_NAME, mitk::IPersistenceService* persistenceService )\
        {\
            m_##THE_CLASS_NAME = _##THE_CLASS_NAME;\
            m_PersistenceService = persistenceService;\
            if( m_PersistenceService )\
                m_PersistenceService->AddPropertyListReplacedObserver(this);\
        }\
    private:\
        THE_CLASS_NAME * m_##THE_CLASS_NAME;\
        mitk::IPersistenceService* m_PersistenceService;\
    };\
    MyPropertyListReplacedObserver m_MyPropertyListReplacedObserver;\
    void InitializePropertyListReplacedObserver(mitk::IPersistenceService* persistenceService)\
    {\
        static bool observerInitialized = false;\
        if( observerInitialized == false && persistenceService )\
        {\
            m_MyPropertyListReplacedObserver.Initialize( this, persistenceService );\
            observerInitialized = true;\
        }\
    }

#define PERSISTENCE_CREATE(THE_CLASS_NAME, ID_MEMBER_NAME, PARAM_MEMBER_NAME)\
    PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
        propList->Set( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
        propList->Get( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)

#define PERSISTENCE_CREATE2(THE_CLASS_NAME, ID_MEMBER_NAME, PARAM_MEMBER_NAME, PARAM2_MEMBER_NAME)\
    PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
    propList->Set( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Set( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
    propList->Get( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Get( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)

#define PERSISTENCE_CREATE3(THE_CLASS_NAME, ID_MEMBER_NAME, PARAM_MEMBER_NAME, PARAM2_MEMBER_NAME, PARAM3_MEMBER_NAME)\
    PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
    propList->Set( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Set( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Set( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
    propList->Get( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Get( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Get( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)

#define PERSISTENCE_CREATE4(THE_CLASS_NAME, ID_MEMBER_NAME, PARAM_MEMBER_NAME, PARAM2_MEMBER_NAME, PARAM3_MEMBER_NAME, PARAM4_MEMBER_NAME)\
    PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
    propList->Set( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Set( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Set( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Set( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
    propList->Get( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Get( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Get( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Get( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)

#define PERSISTENCE_CREATE5(THE_CLASS_NAME, ID_MEMBER_NAME, PARAM_MEMBER_NAME, PARAM2_MEMBER_NAME, PARAM3_MEMBER_NAME, PARAM4_MEMBER_NAME, PARAM5_MEMBER_NAME)\
    PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
    propList->Set( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Set( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Set( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Set( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    propList->Set( #PARAM5_MEMBER_NAME, PARAM5_MEMBER_NAME );\
    PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
    propList->Get( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Get( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Get( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Get( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    propList->Get( #PARAM5_MEMBER_NAME, PARAM5_MEMBER_NAME );\
    PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)

#define PERSISTENCE_CREATE6(THE_CLASS_NAME, ID_MEMBER_NAME, PARAM_MEMBER_NAME, PARAM2_MEMBER_NAME, PARAM3_MEMBER_NAME, PARAM4_MEMBER_NAME, PARAM5_MEMBER_NAME, PARAM6_MEMBER_NAME)\
    PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
    propList->Set( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Set( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Set( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Set( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    propList->Set( #PARAM5_MEMBER_NAME, PARAM5_MEMBER_NAME );\
    propList->Set( #PARAM6_MEMBER_NAME, PARAM6_MEMBER_NAME );\
    PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
    propList->Get( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Get( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Get( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Get( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    propList->Get( #PARAM5_MEMBER_NAME, PARAM5_MEMBER_NAME );\
    propList->Get( #PARAM6_MEMBER_NAME, PARAM6_MEMBER_NAME );\
    PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)

#define PERSISTENCE_CREATE7(THE_CLASS_NAME, ID_MEMBER_NAME, PARAM_MEMBER_NAME, PARAM2_MEMBER_NAME, PARAM3_MEMBER_NAME, PARAM4_MEMBER_NAME, PARAM5_MEMBER_NAME, PARAM6_MEMBER_NAME, PARAM7_MEMBER_NAME)\
    PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
    propList->Set( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Set( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Set( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Set( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    propList->Set( #PARAM5_MEMBER_NAME, PARAM5_MEMBER_NAME );\
    propList->Set( #PARAM6_MEMBER_NAME, PARAM6_MEMBER_NAME );\
    propList->Set( #PARAM7_MEMBER_NAME, PARAM7_MEMBER_NAME );\
    PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
    propList->Get( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Get( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Get( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Get( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    propList->Get( #PARAM5_MEMBER_NAME, PARAM5_MEMBER_NAME );\
    propList->Get( #PARAM6_MEMBER_NAME, PARAM6_MEMBER_NAME );\
    propList->Get( #PARAM7_MEMBER_NAME, PARAM7_MEMBER_NAME );\
    PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)

#define PERSISTENCE_CREATE8(THE_CLASS_NAME, ID_MEMBER_NAME, PARAM_MEMBER_NAME, PARAM2_MEMBER_NAME, PARAM3_MEMBER_NAME, PARAM4_MEMBER_NAME, PARAM5_MEMBER_NAME, PARAM6_MEMBER_NAME, PARAM7_MEMBER_NAME, PARAM8_MEMBER_NAME)\
    PERSISTENCE_MACRO_START_PART(ID_MEMBER_NAME)\
    propList->Set( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Set( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Set( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Set( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    propList->Set( #PARAM5_MEMBER_NAME, PARAM5_MEMBER_NAME );\
    propList->Set( #PARAM6_MEMBER_NAME, PARAM6_MEMBER_NAME );\
    propList->Set( #PARAM7_MEMBER_NAME, PARAM7_MEMBER_NAME );\
    propList->Set( #PARAM8_MEMBER_NAME, PARAM8_MEMBER_NAME );\
    PERSISTENCE_MACRO_MIDDLE_PART(ID_MEMBER_NAME)\
    propList->Get( #PARAM_MEMBER_NAME, PARAM_MEMBER_NAME );\
    propList->Get( #PARAM2_MEMBER_NAME, PARAM2_MEMBER_NAME );\
    propList->Get( #PARAM3_MEMBER_NAME, PARAM3_MEMBER_NAME );\
    propList->Get( #PARAM4_MEMBER_NAME, PARAM4_MEMBER_NAME );\
    propList->Get( #PARAM5_MEMBER_NAME, PARAM5_MEMBER_NAME );\
    propList->Get( #PARAM6_MEMBER_NAME, PARAM6_MEMBER_NAME );\
    propList->Get( #PARAM7_MEMBER_NAME, PARAM7_MEMBER_NAME );\
    propList->Get( #PARAM8_MEMBER_NAME, PARAM8_MEMBER_NAME );\
    PERSISTENCE_MACRO_END_PART(THE_CLASS_NAME, ID_MEMBER_NAME)

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPersistenceService, "org.mitk.services.IPersistenceService")

#endif
