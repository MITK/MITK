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
#include <Poco\Path.h>

std::string mitk::PersistenceService::CreateDefaultFileName()
{
    std::string homeDir = Poco::Path::home();
    std::string file = homeDir + "PersistenceService.mitk";
    return file;
}

const std::string mitk::PersistenceService::DEFAULT_FILE_NAME(CreateDefaultFileName());

const std::string mitk::PersistenceService::PERSISTENCE_PROPERTY_NAME("PersistenceNode");

const std::string mitk::PersistenceService::PERSISTENCE_PROPERTYLIST_NAME("PersistenceService");

const std::string mitk::PersistenceService::ID_PROPERTY_NAME("Id");


mitk::PersistenceService::PersistenceService()
  : m_AutoLoadAndSave( false ), m_SceneIO( SceneIO::New() )
{
    {
        MITK_DEBUG("mitk::PersistenceService") << "constructing PersistenceService";
    }

    MITK_DEBUG("mitk::PersistenceService") << "loading PersistenceService personal persitent data";
    this->Load( DEFAULT_FILE_NAME );
    std::string id = PERSISTENCE_PROPERTYLIST_NAME;
    mitk::PropertyList::Pointer propList = this->GetPropertyList( id );
    propList->GetBoolProperty("m_AutoLoadAndSave", m_AutoLoadAndSave);

    if( m_AutoLoadAndSave == false )
    {
        MITK_DEBUG("mitk::PersistenceService") << "autoloading was not wished. clearing data we got so far.";
        m_PropertyLists.clear();
    }
}

mitk::PersistenceService::~PersistenceService()
{
  MITK_DEBUG("mitk::PersistenceService") << "destructing PersistenceService";
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

void mitk::PersistenceService::ClonePropertyList( mitk::PropertyList* from, mitk::PropertyList* to )
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

bool mitk::PersistenceService::Save(const std::string& fileName)
{
    bool save = false;
    mitk::StandaloneDataStorage::Pointer tempDs = mitk::StandaloneDataStorage::New();

    std::map<std::string, mitk::PropertyList::Pointer>::iterator it = m_PropertyLists.begin();
    while( it != m_PropertyLists.end() )
    {
        mitk::DataNode::Pointer newNode = mitk::DataNode::New();

        this->ClonePropertyList( (*it).second, newNode->GetPropertyList() );

        newNode->SetBoolProperty( PERSISTENCE_PROPERTY_NAME.c_str(), true );
        newNode->SetName( (*it).first );
        newNode->SetStringProperty(ID_PROPERTY_NAME.c_str(),(*it).first.c_str() );

        tempDs->Add(newNode);
        ++it;
    }

    mitk::NodePredicateProperty::Pointer pred =
        mitk::NodePredicateProperty::New(PERSISTENCE_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(true));

    mitk::DataStorage::SetOfObjects::ConstPointer rs = tempDs->GetSubset(pred);

    std::string theFile = fileName;
    if(theFile.empty())
        theFile = DEFAULT_FILE_NAME;
    return m_SceneIO->SaveScene( rs, tempDs, theFile );
}

bool mitk::PersistenceService::Load(const std::string& fileName)
{
    bool load = false;

    std::string theFile = fileName;
    if(theFile.empty())
        theFile = DEFAULT_FILE_NAME;
    DataStorage::Pointer ds = m_SceneIO->LoadScene( theFile );
    load = (m_SceneIO->GetFailedNodes() == 0 || m_SceneIO->GetFailedNodes()->size() == 0) && (m_SceneIO->GetFailedNodes() == 0 || m_SceneIO->GetFailedProperties()->IsEmpty());
    if( !load )
    {
        MITK_DEBUG("mitk::PersistenceService") << "loading of scene files failed";
        return load;
    }

    DataStorage::SetOfObjects::ConstPointer allNodes = ds->GetAll();
    for ( mitk::DataStorage::SetOfObjects::const_iterator sourceIter = allNodes->begin();
        sourceIter != allNodes->end();
        ++sourceIter )
    {
        mitk::DataNode* node = *sourceIter;
        bool isPersistenceNode = false;
        node->GetBoolProperty( PERSISTENCE_PROPERTY_NAME.c_str(), isPersistenceNode );

        if( isPersistenceNode )
        {
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
