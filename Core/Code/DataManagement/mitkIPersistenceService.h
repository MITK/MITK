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
#include <usServiceInterface.h>
#include "mitkDataStorage.h"

namespace mitk
{
    ///
    /// The central service for the persistence module
    /// Basic idea is to create PropertyLists with a unique id using AddPropertyList(). A consumer
    /// of this interface can write arbitrary information into this propertylist
    /// Calling Save() and Load() will cause the Service to save and load the current set of propertlists from
    /// a file in the user directory.
    /// Using SetAutoLoadAndSave(true) will cause the service to load/save the property lists at application
    /// start/stop.
    /// Moreover, depending on the backend type, the service is connected to the SceneSerialization module, i.e.
    /// the user will be asked whether to save/load the propertlists in/from the current ".mitk" file that is selected
    /// by the user.
    ///
    class MITK_CORE_EXPORT IPersistenceService
    {
    public:
        ///
        /// Set a default directory for storage
        ///
        static std::string CreateDefaultFileName();
        ///
        /// Get the default directory for storage
        ///
        static void SetDefaultPersistenceFile(const std::string& defaultPersistenceFile);
        ///
        /// Set a default directory for storage
        ///
        static std::string GetDefaultPersistenceFile();
        ///
        /// If PropertyList with the given id exists, returns it. Otherwise creates a new one and returns it.
        /// If id is empty a UUID will be created and set on the variable
        /// If existed was passed, it is true if the PropertyList with that id existed, false otherwise
        /// \return a valid PropertyList with a StringProperty "Id" containing the passed id
        ///
        virtual mitk::PropertyList::Pointer GetPropertyList( std::string& id, bool* existed=0 ) = 0;
        ///
        /// \return The name of the Bool Property that specifies whether a DataNode is a Node carrying Persistence PropertyLists
        ///
        virtual std::string GetPersistenceNodePropertyName() const = 0;
        ///
        /// Creates a vector of DataNodes that contain all PropertyLists. Additionally, the DataNodes
        /// will have the property name set to the PropertyList's id and a BoolProperty equal to GetPersistenceNodePropertyName() set to true
        /// \return vector of DataNodes with the described attributes
        ///
        virtual DataStorage::SetOfObjects::Pointer GetDataNodes() const = 0;
        ///
        /// Searches storage for persistent DataNodes, extracts and inserts the appended property lists to this service
        /// \return true if at least one node was found from which a PropertyList could be restored
        ///
        virtual bool RestorePropertyListsFromPersistentDataNodes(DataStorage* storage) = 0;
        ///
        /// Save the current PropertyLists to fileName. If fileName is empty, a special file in the users home directory will be used.
        /// if appendchanges is true, the file will not replaced but first loaded, then overwritten and then replaced
        /// \return false if an error occured (cannot write to file), true otherwise
        ///
        virtual bool Save(const std::string& fileName="", bool appendChanges=false) = 0;
        ///
        /// Load PropertyLists from fileName. If fileName is empty, a special file in the users home directory will be used.
        /// *ATTENTION*: If there are PropertyLists with the same id contained in the file, existing PropertyLists will be overwritten!
        /// \see AddPropertyListReplacedObserver()
        /// \return false if an error occured (cannot load from file), true otherwise
        ///
        virtual bool Load(const std::string& fileName="") = 0;
        ///
        /// Using SetAutoLoadAndSave(true) will cause the service to load/save the property lists at application
        /// start/stop.
        ///
        virtual void SetAutoLoadAndSave(bool autoLoadAndSave) = 0;
        ///
        /// adds a observer which is informed if a propertyList gets replaced during a Load() procedure
        ///
        virtual void AddPropertyListReplacedObserver( PropertyListReplacedObserver* observer ) = 0;
        ///
        /// removes a specific observer
        ///
        virtual void RemovePropertyListReplacedObserver( PropertyListReplacedObserver* observer ) = 0;
        ///
        /// nothing to do here
        ///
        virtual ~IPersistenceService();
    private:
        static std::string m_DefaultPersistenceFile;
    };
}

/// MACROS FOR AUTOMATIC SAVE FUNCTION
#define PERSISTENCE_GET_MODULE_CONTEXT\
    us::ModuleContext* context = GetModuleContext();

#define PERSISTENCE_GET_SERVICE\
    PERSISTENCE_GET_MODULE_CONTEXT\
    us::ServiceReference<mitk::IPersistenceService> persistenceServiceRef = context->GetServiceReference<mitk::IPersistenceService>();\
    mitk::IPersistenceService* persistenceService = dynamic_cast<mitk::IPersistenceService*> ( context->GetService<mitk::IPersistenceService>(persistenceServiceRef) );

#define PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
bool Save(const std::string& fileName="") { \
    PERSISTENCE_GET_SERVICE\
    bool noError = persistenceService != 0;\
    mitk::PropertyList::Pointer propList;\
    if( noError ) {\
        propList = persistenceService->GetPropertyList(IdMemberName);

#define PERSISTENCE_CREATE_SAVE_END\
    }\
    noError = persistenceService->Save(fileName);\
    return noError;\
}

#define PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
void SetId( const std::string& ____id ) { IdMemberName = ____id; };\
std::string GetId() { return IdMemberName; };\
bool Load(const std::string& fileName="") {\
    PERSISTENCE_GET_SERVICE\
    bool noError = persistenceService != 0 && persistenceService->Load(fileName);\
    if( noError ) {\
    mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(IdMemberName);

#define PERSISTENCE_CREATE_LOAD_END\
    }\
    return noError;\
}

#define PERSISTENCE_CREATE(IdMemberName, ParamMemberName)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( #ParamMemberName, ParamMemberName );\
    PERSISTENCE_CREATE_SAVE_END\
    PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
        noError = propList->Get( #ParamMemberName, ParamMemberName );\
    PERSISTENCE_CREATE_LOAD_END

#define PERSISTENCE_CREATE2(IdMemberName, ParamMemberName, Param2MemberName)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( #ParamMemberName, ParamMemberName );\
        propList->Set( #Param2MemberName, Param2MemberName );\
    PERSISTENCE_CREATE_SAVE_END\
    PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
        noError = propList->Get( #ParamMemberName, ParamMemberName );\
        if(noError)\
        noError = propList->Get( #Param2MemberName, Param2MemberName );\
    PERSISTENCE_CREATE_LOAD_END

#define PERSISTENCE_CREATE3(IdMemberName, ParamMemberName, Param2MemberName, Param3MemberName)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( #ParamMemberName, ParamMemberName );\
        propList->Set( #Param2MemberName, Param2MemberName );\
        propList->Set( #Param3MemberName, Param3MemberName );\
    PERSISTENCE_CREATE_SAVE_END\
    PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
        noError = propList->Get( #ParamMemberName, ParamMemberName );\
        if(noError)\
        noError = propList->Get( #Param2MemberName, Param2MemberName );\
        if(noError)\
        noError = propList->Get( #Param3MemberName, Param3MemberName );\
    PERSISTENCE_CREATE_LOAD_END

#define PERSISTENCE_CREATE4(IdMemberName, ParamMemberName, Param2MemberName, Param3MemberName, Param4MemberName)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( #ParamMemberName, ParamMemberName );\
        propList->Set( #Param2MemberName, Param2MemberName );\
        propList->Set( #Param3MemberName, Param3MemberName );\
        propList->Set( #Param4MemberName, Param4MemberName );\
    PERSISTENCE_CREATE_SAVE_END\
    PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
        noError = propList->Get( #ParamMemberName, ParamMemberName );\
        if(noError)\
        noError = propList->Get( #Param2MemberName, Param2MemberName );\
        if(noError)\
        noError = propList->Get( #Param3MemberName, Param3MemberName );\
        if(noError)\
        noError = propList->Get( #Param4MemberName, Param4MemberName );\
    PERSISTENCE_CREATE_LOAD_END

US_DECLARE_SERVICE_INTERFACE(mitk::IPersistenceService, "org.mitk.services.IPersistenceService")

#endif
