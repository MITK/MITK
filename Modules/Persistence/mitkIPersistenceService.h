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
#include "MitkExtExports.h"
#include "mitkImage.h"
//for microservices
#include <usServiceInterface.h>
#include "mitkSurface.h"
#include <vector>

namespace mitk
{
    ///
    /// a PropertyListReplacedObserver gets informed as soon as a PropertyList with a given id was replaced during a Load() process
    /// \see IPersistenceService::AddPropertyListReplacedObserver()
    ///
    class PropertyListReplacedObserver
    {
    public:
        ///
        /// will be called *before* the propertyList gets replaced with new contents, i.e. propertyList still contains the old values
        ///
        virtual void BeforePropertyListReplaced( const std::string& id, mitk::PropertyList* propertyList ) {};
        ///
        /// will be called *after* the propertyList gets replaced with new contents, i.e. propertyList contains the new values
        ///
        virtual void AfterPropertyListReplaced( const std::string& id, mitk::PropertyList* propertyList ) {};
    };

    ///
    /// The central service for the persistence module
    /// Basic idea is to create PropertyLists with a unique id using GetPropertyList(). A consumer
    /// of this interface can write arbitrary information into this propertylist
    /// Calling Save() and Load() will cause the Service to save and load the current set of propertlists from
    /// a file in the user directory.
    /// Using SetAutoLoadAndSave(true) will cause the service to load/save the property lists at application
    /// start/stop.
    /// Moreover, depending on the backend type, the service is connected to the SceneSerialization module, i.e.
    /// the user will be asked whether to save/load the propertlists in/from the current ".mitk" file that is selected
    /// by the user.
    ///
    class MitkExt_EXPORT IPersistenceService
    {
    public:
        ///
        /// If PropertyList with the given id exists, returns it. Otherwise creates a new one and returns it.
        /// \return false if id already exists, true otherwise
        ///
        virtual bool AddPropertyList( const std::string& id, mitk::PropertyList* propertyList ) = 0;
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
        virtual void SetAutoLoadAndSave(bool autoLoadAndSave);
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

US_DECLARE_SERVICE_INTERFACE(mitk::IPersistenceService, "org.mitk.services.IPersistenceService")

#endif
