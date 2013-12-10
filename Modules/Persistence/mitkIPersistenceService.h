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
//for microservices
#include <usServiceInterface.h>

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
    class Persistence_EXPORT IPersistenceService
    {
    public:
        ///
        /// If PropertyList with the given id exists, returns it. Otherwise creates a new one and returns it.
        /// If id is empty a UUID will be created and set on the variable
        /// If existed was passed, it is true if the PropertyList with that id existed, false otherwise
        /// \return a valid PropertyList with a StringProperty "Id" containing the passed id
        ///
        virtual mitk::PropertyList::Pointer GetPropertyList( std::string& id, bool* existed=0 ) = 0;
        ///
        /// Save the current PropertyLists to fileName. If fileName is empty, a special file in the users home directory will be used.
        /// \return false if an error occured (cannot write to file), true otherwise
        ///
        virtual bool Save(const std::string& fileName="") = 0;
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
bool Save() { \
    PERSISTENCE_GET_SERVICE\
    bool noError = persistenceService != 0;\
    mitk::PropertyList::Pointer propList;\
    if( noError ) {\
        propList = persistenceService->GetPropertyList(IdMemberName);

#define PERSISTENCE_CREATE_SAVE_END\
    }\
    noError = persistenceService->Save();\
    return noError;\
}

#define PERSISTENCE_CREATE_SAVE(IdMemberName, ParamMemberName)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( "ParamMemberName", ParamMemberName );\
    PERSISTENCE_CREATE_SAVE_END

#define PERSISTENCE_CREATE_SAVE2(IdMemberName, ParamMemberName, ParamMember2Name)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( "ParamMemberName", ParamMemberName );\
        propList->Set( "ParamMember2Name", ParamMember2Name );\
    PERSISTENCE_CREATE_SAVE_END

#define PERSISTENCE_CREATE_SAVE3(IdMemberName, ParamMemberName, ParamMember2Name, ParamMember3Name)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( "ParamMemberName", ParamMemberName );\
        propList->Set( "ParamMember2Name", ParamMember2Name );\
        propList->Set( "ParamMember3Name", ParamMember3Name );\
    PERSISTENCE_CREATE_SAVE_END

#define PERSISTENCE_CREATE_SAVE4(IdMemberName, ParamMemberName, ParamMember2Name, ParamMember3Name, ParamMember4Name)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( "ParamMemberName", ParamMemberName );\
        propList->Set( "ParamMember2Name", ParamMember2Name );\
        propList->Set( "ParamMember3Name", ParamMember3Name );\
        propList->Set( "ParamMember4Name", ParamMember4Name );\
    PERSISTENCE_CREATE_SAVE_END

#define PERSISTENCE_CREATE_SAVE5(IdMemberName, ParamMemberName, ParamMember2Name, ParamMember3Name, Param4Name, ParamMember5Name)\
    PERSISTENCE_CREATE_SAVE_START(IdMemberName)\
        propList->Set( "ParamMemberName", ParamMemberName );\
        propList->Set( "ParamMember2Name", ParamMember2Name );\
        propList->Set( "ParamMember3Name", ParamMember3Name );\
        propList->Set( "ParamMember4Name", ParamMember4Name );\
        propList->Set( "ParamMember5Name", ParamMember5Name );\
    PERSISTENCE_CREATE_SAVE_END

/// MACROS FOR AUTOMATIC LOAD FUNCTION
#define PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
bool Load() {\
    PERSISTENCE_GET_SERVICE\
    bool noError = persistenceService != 0 && persistenceService->Load();\
    if( noError ) {

#define PERSISTENCE_CREATE_LOAD_END\
    }\
        return noError;\
}

#define PERSISTENCE_CREATE_LOAD(IdMemberName, ParamMemberName)\
    PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
        mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(IdMemberName);\
        noError = propList->Get( "ParamMemberName", ParamMemberName );\
    PERSISTENCE_CREATE_LOAD_END

#define PERSISTENCE_CREATE_LOAD2(IdMemberName, ParamMemberName, Param2MemberName)\
    PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
        mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(IdMemberName);\
        noError = propList->Get( "ParamMemberName", ParamMemberName );\
        if(noError)\
            noError = propList->Get( "Param2MemberName", Param2MemberName );\
    PERSISTENCE_CREATE_LOAD_END

#define PERSISTENCE_CREATE_LOAD3(IdMemberName, ParamMemberName, Param2MemberName, Param3MemberName)\
    PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
        mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(IdMemberName);\
        noError = propList->Get( "ParamMemberName", ParamMemberName );\
        if(noError)\
            noError = propList->Get( "Param2MemberName", Param2MemberName );\
        if(noError)\
            noError = propList->Get( "Param3MemberName", Param3MemberName );\
    PERSISTENCE_CREATE_LOAD_END

#define PERSISTENCE_CREATE_LOAD4(IdMemberName, ParamMemberName, Param2MemberName, Param3MemberName, Param4MemberName)\
    PERSISTENCE_CREATE_LOAD_START(IdMemberName)\
        mitk::PropertyList::Pointer propList = persistenceService->GetPropertyList(IdMemberName);\
        noError = propList->Get( "ParamMemberName", ParamMemberName );\
        if(noError)\
            noError = propList->Get( "Param2MemberName", Param2MemberName );\
        if(noError)\
            noError = propList->Get( "Param3MemberName", Param3MemberName );\
        if(noError)\
            noError = propList->Get( "Param4MemberName", Param4MemberName );\
    PERSISTENCE_CREATE_LOAD_END

US_DECLARE_SERVICE_INTERFACE(mitk::IPersistenceService, "org.mitk.services.IPersistenceService")

#endif
