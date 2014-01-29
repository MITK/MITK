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

#include "mitkPersistenceService.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkUIDGenerator.h"
#include "mitkNodePredicateProperty.h"
#include "mitkProperties.h"
#include "usModuleContext.h"
#include "usGetModuleContext.h"
#include <itksys/SystemTools.hxx>

const std::string mitk::PersistenceService::PERSISTENCE_PROPERTY_NAME("PersistenceNode");

const std::string mitk::PersistenceService::PERSISTENCE_PROPERTYLIST_NAME("PersistenceService");

const std::string mitk::PersistenceService::ID_PROPERTY_NAME("id");


mitk::PersistenceService::PersistenceService()
: m_AutoLoadAndSave( true ), m_SceneIO( SceneIO::New() ), m_PropertyListsXmlFileReaderAndWriter( PropertyListsXmlFileReaderAndWriter::New() )
{
}

void mitk::PersistenceService::Clear()
{
    m_PropertyLists.clear();
    m_FileNamesToModifiedTimes.clear();
}

mitk::PersistenceService::~PersistenceService()
{
  MITK_DEBUG("mitk::PersistenceService") << "destructing PersistenceService";
}

std::string mitk::PersistenceService::GetDefaultPersistenceFile() const
{
    std::string file = "PersistentData.mitk";
    us::ModuleContext* context = us::GetModuleContext();
    std::string contextDataFile = context->GetDataFile("PersistentData.mitk");
    if( !contextDataFile.empty() )
    {
        file = contextDataFile;
    }
    return file;
}

mitk::PropertyList::Pointer mitk::PersistenceService::GetPropertyList( std::string& id, bool* existed )
{
    mitk::PropertyList::Pointer propList;

    if( id.empty() )
    {
        UIDGenerator uidGen;
        id = uidGen.GetUID();
    }

    std::map<std::string, mitk::PropertyList::Pointer>::iterator it = m_PropertyLists.find( id );
    if( it == m_PropertyLists.end() )
    {
        propList = PropertyList::New();
        m_PropertyLists[id] = propList;
        if( existed )
            *existed = false;
    }
    else
    {
        propList = (*it).second;
        if( existed )
            *existed = true;
    }

    return propList;
}

void mitk::PersistenceService::ClonePropertyList( mitk::PropertyList* from, mitk::PropertyList* to ) const
{
    to->Clear();

    const std::map< std::string, BaseProperty::Pointer>* propMap = from->GetMap();
    std::map< std::string, BaseProperty::Pointer>::const_iterator propMapIt = propMap->begin();
    while( propMapIt != propMap->end() )
    {
        mitk::BaseProperty::Pointer clonedProp = (*propMapIt).second->Clone();
        to->SetProperty( (*propMapIt).first, clonedProp );
        ++propMapIt;
    }
}

bool mitk::PersistenceService::Save(const std::string& fileName, bool appendChanges)
{
    bool save = false;
    std::string theFile = fileName;
    if(theFile.empty())
        theFile = PersistenceService::GetDefaultPersistenceFile();

    bool xmlFile = false;
    if( itksys::SystemTools::GetFilenameLastExtension(theFile.c_str()) == ".xml" )
        xmlFile = true;

    mitk::DataStorage::Pointer tempDs;
    if(appendChanges && xmlFile == false)
    {
        if( itksys::SystemTools::FileExists(theFile.c_str()) )
        {

            bool load = false;
            DataStorage::Pointer ds = m_SceneIO->LoadScene( theFile );
            load = (m_SceneIO->GetFailedNodes() == 0 || m_SceneIO->GetFailedNodes()->size() == 0) && (m_SceneIO->GetFailedNodes() == 0 || m_SceneIO->GetFailedProperties()->IsEmpty());
            if( !load )
                return false;

            tempDs = ds;

        }
    }
    else
    {
        tempDs = mitk::StandaloneDataStorage::New();
        if( xmlFile && appendChanges && itksys::SystemTools::FileExists(theFile.c_str()) )
        {
            if( !m_PropertyListsXmlFileReaderAndWriter->ReadLists( theFile, m_PropertyLists ) )
                return false;
        }
    }

    DataStorage::SetOfObjects::Pointer rs = DataStorage::SetOfObjects::New();

    if( xmlFile )
    {
        save = m_PropertyListsXmlFileReaderAndWriter->WriteLists(theFile, m_PropertyLists);
    }
    else
    {
        save = m_SceneIO->SaveScene( rs.GetPointer(), tempDs, theFile );
    }
    if( save )
    {
        long int currentModifiedTime = itksys::SystemTools::ModifiedTime( theFile.c_str() );
        m_FileNamesToModifiedTimes[theFile] = currentModifiedTime;
    }
    return save;
}

bool mitk::PersistenceService::Load(const std::string& fileName, bool enforceReload)
{
    bool load = false;

    std::string theFile = fileName;
    if(theFile.empty())
        theFile = PersistenceService::GetDefaultPersistenceFile();

    if( !itksys::SystemTools::FileExists(theFile.c_str()) )
        return false;

    bool xmlFile = false;
    if( itksys::SystemTools::GetFilenameLastExtension(theFile.c_str()) == ".xml" )
        xmlFile = true;

    if( enforceReload == false )
    {
        bool loadTheFile = true;
        std::map<std::string, long int>::iterator it = m_FileNamesToModifiedTimes.find( theFile );

        long int currentModifiedTime = itksys::SystemTools::ModifiedTime( theFile.c_str() );
        if( it != m_FileNamesToModifiedTimes.end() )
        {
            long int knownModifiedTime = (*it).second;
            if( knownModifiedTime >= currentModifiedTime )
            {
                loadTheFile = false;
            }
        }
        if( loadTheFile )
            m_FileNamesToModifiedTimes[theFile] = currentModifiedTime;
        else
            return true;
    }

    if( xmlFile )
    {
        load = m_PropertyListsXmlFileReaderAndWriter->ReadLists(theFile, m_PropertyLists);
    }
    else
    {
        DataStorage::Pointer ds = m_SceneIO->LoadScene( theFile );
        load = (m_SceneIO->GetFailedNodes() == 0 || m_SceneIO->GetFailedNodes()->size() == 0) && (m_SceneIO->GetFailedNodes() == 0 || m_SceneIO->GetFailedProperties()->IsEmpty());
    }
    if( !load )
    {
        MITK_DEBUG("mitk::PersistenceService") << "loading of scene files failed";
        return load;
    }

    return load;
}

void mitk::PersistenceService::SetAutoLoadAndSave(bool autoLoadAndSave)
{
    m_AutoLoadAndSave = autoLoadAndSave;
    std::string id = PERSISTENCE_PROPERTYLIST_NAME;
    mitk::PropertyList::Pointer propList = this->GetPropertyList( id );
    propList->Set("m_AutoLoadAndSave", m_AutoLoadAndSave);
    this->Save();
}

void mitk::PersistenceService::AddPropertyListReplacedObserver(PropertyListReplacedObserver* observer)
{
    m_PropertyListReplacedObserver.insert( observer );
}

void mitk::PersistenceService::RemovePropertyListReplacedObserver(PropertyListReplacedObserver* observer)
{
    m_PropertyListReplacedObserver.erase( observer );
}

us::ModuleContext* mitk::PersistenceService::GetModuleContext()
{
    return us::GetModuleContext();
}

std::string mitk::PersistenceService::GetPersistenceNodePropertyName() const
{
    return PERSISTENCE_PROPERTY_NAME;
}
mitk::DataStorage::SetOfObjects::Pointer mitk::PersistenceService::GetDataNodes() const
{
    DataStorage::SetOfObjects::Pointer set = DataStorage::SetOfObjects::New();

    std::map<std::string, mitk::PropertyList::Pointer>::const_iterator it = m_PropertyLists.begin();
    while( it != m_PropertyLists.end() )
    {
        mitk::DataNode::Pointer node = mitk::DataNode::New();
        const std::string& name = (*it).first;

        this->ClonePropertyList( (*it).second, node->GetPropertyList() );

        node->SetBoolProperty( PERSISTENCE_PROPERTY_NAME.c_str(), true );
        node->SetName( name );
        node->SetStringProperty(ID_PROPERTY_NAME.c_str(), name.c_str() );

        set->push_back( node );
        ++it;
    }

    return set;
}

bool mitk::PersistenceService::RestorePropertyListsFromPersistentDataNodes( DataStorage* storage )
{
    bool oneFound = false;
    DataStorage::SetOfObjects::ConstPointer allNodes = storage->GetAll();
    for ( mitk::DataStorage::SetOfObjects::const_iterator sourceIter = allNodes->begin();
        sourceIter != allNodes->end();
        ++sourceIter )
    {
        mitk::DataNode* node = *sourceIter;
        bool isPersistenceNode = false;
        node->GetBoolProperty( PERSISTENCE_PROPERTY_NAME.c_str(), isPersistenceNode );

        if( isPersistenceNode )
        {
            oneFound = true;
            MITK_DEBUG("mitk::PersistenceService") << "isPersistenceNode was true";
            std::string name = node->GetName();
            bool existed = false;
            mitk::PropertyList::Pointer propList = this->GetPropertyList( name, &existed );

            if( existed )
            {
                MITK_DEBUG("mitk::PersistenceService") << "calling replace observer before replacing values";
                std::set<PropertyListReplacedObserver*>::iterator it = m_PropertyListReplacedObserver.begin();
                while( it != m_PropertyListReplacedObserver.end() )
                {
                    (*it)->BeforePropertyListReplaced( name, propList );
                    ++it;
                }
            } // if( existed )

            MITK_DEBUG("mitk::PersistenceService") << "replacing values";

            this->ClonePropertyList(  node->GetPropertyList(), propList );

            propList->SetStringProperty(ID_PROPERTY_NAME.c_str(), name.c_str());

            if( existed )
            {
                MITK_DEBUG("mitk::PersistenceService") << "calling replace observer before replacing values";
                std::set<PropertyListReplacedObserver*>::iterator it = m_PropertyListReplacedObserver.begin();
                while( it != m_PropertyListReplacedObserver.end() )
                {
                    (*it)->AfterPropertyListReplaced( name, propList );
                    ++it;
                }
            } // if( existed )
        } // if( isPersistenceNode )
    } // for ( mitk::DataStorage::SetOfObjects::const_iterator sourceIter = allNodes->begin(); ...

    return oneFound;
}

bool mitk::PersistenceService::GetAutoLoadAndSave() const
{
    return m_AutoLoadAndSave;
}

bool mitk::PersistenceService::RemovePropertyList( std::string& id )
{
    std::map<std::string, mitk::PropertyList::Pointer>::iterator it = m_PropertyLists.find( id );
    if( it != m_PropertyLists.end() )
    {
        m_PropertyLists.erase(it);
        return true;
    }
    return false;
}
