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
#ifndef mitkPersistenceService_h
#define mitkPersistenceService_h

#include "mitkIPersistenceService.h"
#include "mitkPropertyListsXmlFileReaderAndWriter.h"
#include <itkLightObject.h>
#include "mitkSceneIO.h"

namespace mitk
{
    ///
    /// implementation of the IPersistenceService
    /// \see IPersistenceService
    class PersistenceService: public itk::LightObject, public mitk::IPersistenceService
    {
    public:
        static const std::string PERSISTENCE_PROPERTY_NAME;
        static const std::string PERSISTENCE_PROPERTYLIST_NAME;
        static const std::string ID_PROPERTY_NAME;
        static us::ModuleContext* GetModuleContext();

        PersistenceService();

        ~PersistenceService();

        std::string GetDefaultPersistenceFile() const;

        mitk::PropertyList::Pointer GetPropertyList( std::string& id, bool* existed=0 );

        bool RemovePropertyList( std::string& id );

        std::string GetPersistenceNodePropertyName() const;

        DataStorage::SetOfObjects::Pointer GetDataNodes() const;

        bool Save(const std::string& fileName="", bool appendChanges=false);

        bool Load(const std::string& fileName="", bool enforeReload=true);

        void SetAutoLoadAndSave(bool autoLoadAndSave);

        bool GetAutoLoadAndSave() const;

        void AddPropertyListReplacedObserver( PropertyListReplacedObserver* observer );

        void RemovePropertyListReplacedObserver( PropertyListReplacedObserver* observer );

        bool RestorePropertyListsFromPersistentDataNodes(DataStorage* storage);

        void Clear();
    private:
        void ClonePropertyList( mitk::PropertyList* from, mitk::PropertyList* to ) const;
        std::map<std::string, mitk::PropertyList::Pointer> m_PropertyLists;
        bool m_AutoLoadAndSave;
        std::set<PropertyListReplacedObserver*> m_PropertyListReplacedObserver;
        SceneIO::Pointer m_SceneIO;
        PropertyListsXmlFileReaderAndWriter::Pointer m_PropertyListsXmlFileReaderAndWriter;
        std::map<std::string, long int> m_FileNamesToModifiedTimes;
    };
}
#endif
