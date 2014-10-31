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
#include <MitkPersistenceExports.h>

namespace mitk
{
    ///
    /// implementation of the IPersistenceService
    /// \see IPersistenceService
    class MitkPersistence_EXPORT PersistenceService: public itk::LightObject, public mitk::IPersistenceService
    {
    public:
        static std::string GetPersistencePropertyName();

        static std::string GetPersistencePropertyListName();

        static void LoadModule();
        static us::ModuleContext* GetModuleContext();

        PersistenceService();

        ~PersistenceService();

        std::string GetDefaultPersistenceFile();

        mitk::PropertyList::Pointer GetPropertyList( std::string& id, bool* existed=0 );

        bool RemovePropertyList( std::string& id );

        std::string GetPersistenceNodePropertyName();

        DataStorage::SetOfObjects::Pointer GetDataNodes(DataStorage* ds=0);

        bool Save(const std::string& fileName="", bool appendChanges=false);

        bool Load(const std::string& fileName="", bool enforeReload=true);

        void SetAutoLoadAndSave(bool autoLoadAndSave);

        bool GetAutoLoadAndSave();

        void AddPropertyListReplacedObserver( PropertyListReplacedObserver* observer );

        void RemovePropertyListReplacedObserver( PropertyListReplacedObserver* observer );

        bool RestorePropertyListsFromPersistentDataNodes(const DataStorage* storage);

        void Clear();

        void Unitialize();
    private:
        void ClonePropertyList( mitk::PropertyList* from, mitk::PropertyList* to ) const;
        void Initialize();
        std::map<std::string, mitk::PropertyList::Pointer> m_PropertyLists;
        bool m_AutoLoadAndSave;
        std::set<PropertyListReplacedObserver*> m_PropertyListReplacedObserver;
        SceneIO::Pointer m_SceneIO;
        PropertyListsXmlFileReaderAndWriter::Pointer m_PropertyListsXmlFileReaderAndWriter;
        std::map<std::string, long int> m_FileNamesToModifiedTimes;
        bool m_Initialized;
        bool m_InInitialized;
    };
}
#endif
