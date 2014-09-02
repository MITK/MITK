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

std::string mitk::PersistenceService::GetPersistencePropertyName()
{
  return "PersistenceNode";
}

std::string mitk::PersistenceService::GetPersistencePropertyListName()
{
  return "PersistenceService";
}

mitk::PersistenceService::PersistenceService()
: m_AutoLoadAndSave( true ), m_Initialized(false), m_InInitialized(false)
{
}

void mitk::PersistenceService::Clear()
{
  this->Initialize();
  m_PropertyLists.clear();
  m_FileNamesToModifiedTimes.clear();
}

mitk::PersistenceService::~PersistenceService()
{
  MITK_DEBUG("mitk::PersistenceService") << "destructing PersistenceService";
}

std::string mitk::PersistenceService::GetDefaultPersistenceFile()
{
  this->Initialize();
  std::string file = "PersistentData.xml";
  us::ModuleContext* context = us::GetModuleContext();
  std::string contextDataFile = context->GetDataFile(file);

  if( !contextDataFile.empty() )
  {
      file = contextDataFile;
  }
  return file;
}

mitk::PropertyList::Pointer mitk::PersistenceService::GetPropertyList( std::string& id, bool* existed )
{
  this->Initialize();
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
  this->Initialize();
  bool save = false;
  std::string theFile = fileName;
  if(theFile.empty())
      theFile = PersistenceService::GetDefaultPersistenceFile();

  std::string thePath = itksys::SystemTools::GetFilenamePath( theFile.c_str() );
  if( !thePath.empty() && !itksys::SystemTools::FileExists( thePath.c_str() ) )
  {
    if( !itksys::SystemTools::MakeDirectory( thePath.c_str() ) )
    {
      MITK_ERROR("PersistenceService") << "Could not create " << thePath;
      return false;
    }
  }

  bool createFile = !itksys::SystemTools::FileExists(theFile.c_str());
  if( !itksys::SystemTools::Touch(theFile.c_str(), createFile) )
  {
    MITK_ERROR("PersistenceService") << "Could not create or write to " << theFile;
    return false;
  }

  bool xmlFile = false;
  if( itksys::SystemTools::GetFilenameLastExtension(theFile.c_str()) == ".xml" )
      xmlFile = true;

  mitk::DataStorage::Pointer tempDs;
  if( appendChanges )
  {
      if(xmlFile == false)
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

      this->RestorePropertyListsFromPersistentDataNodes( tempDs );
  }
  else if( xmlFile == false )
  {
      tempDs = mitk::StandaloneDataStorage::New();
  }

  if( xmlFile )
  {
      save = m_PropertyListsXmlFileReaderAndWriter->WriteLists(theFile, m_PropertyLists);
  }

  else
  {
      DataStorage::SetOfObjects::Pointer sceneNodes = this->GetDataNodes(tempDs);
      if(m_SceneIO.IsNull())
      {
        m_SceneIO = mitk::SceneIO::New();
      }
      save = m_SceneIO->SaveScene( sceneNodes.GetPointer(), tempDs, theFile );
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
  this->Initialize();
  bool load = false;

  std::string theFile = fileName;
  if(theFile.empty())
      theFile = PersistenceService::GetDefaultPersistenceFile();

  MITK_INFO << "Load persistence data from file: " << theFile;

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
    if(m_SceneIO.IsNull())
    {
      m_SceneIO = mitk::SceneIO::New();
    }
      DataStorage::Pointer ds = m_SceneIO->LoadScene( theFile );
      load = (m_SceneIO->GetFailedNodes() == 0 || m_SceneIO->GetFailedNodes()->size() == 0) && (m_SceneIO->GetFailedNodes() == 0 || m_SceneIO->GetFailedProperties()->IsEmpty());
      if( load )
      {
          this->RestorePropertyListsFromPersistentDataNodes( ds );
      }
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
  this->Initialize();
  m_AutoLoadAndSave = autoLoadAndSave;
  //std::string id = PERSISTENCE_PROPERTYLIST_NAME; //see bug 17729
  std::string id = GetPersistencePropertyListName();
  mitk::PropertyList::Pointer propList = this->GetPropertyList( id );
  propList->Set("m_AutoLoadAndSave", m_AutoLoadAndSave);
  this->Save();
}

void mitk::PersistenceService::AddPropertyListReplacedObserver(PropertyListReplacedObserver* observer)
{
  this->Initialize();
  m_PropertyListReplacedObserver.insert( observer );
}

void mitk::PersistenceService::RemovePropertyListReplacedObserver(PropertyListReplacedObserver* observer)
{
  this->Initialize();
  m_PropertyListReplacedObserver.erase( observer );
}

void mitk::PersistenceService::LoadModule()
{
    MITK_INFO << "Persistence Module loaded.";
}

us::ModuleContext* mitk::PersistenceService::GetModuleContext()
{
  return us::GetModuleContext();
}

std::string mitk::PersistenceService::GetPersistenceNodePropertyName()
{
  this->Initialize();
  //return PERSISTENCE_PROPERTY_NAME; //see bug 17729
  return GetPersistencePropertyName();
}
mitk::DataStorage::SetOfObjects::Pointer mitk::PersistenceService::GetDataNodes(mitk::DataStorage* ds)
{
  this->Initialize();
  DataStorage::SetOfObjects::Pointer set = DataStorage::SetOfObjects::New();

  std::map<std::string, mitk::PropertyList::Pointer>::const_iterator it = m_PropertyLists.begin();
  while( it != m_PropertyLists.end() )
  {
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      const std::string& name = (*it).first;

      this->ClonePropertyList( (*it).second, node->GetPropertyList() );


      node->SetName( name );
      MITK_DEBUG << "Persistence Property Name: " << GetPersistencePropertyName().c_str();
      //node->SetBoolProperty( PERSISTENCE_PROPERTY_NAME.c_str() , true ); //see bug 17729
      node->SetBoolProperty( GetPersistencePropertyName().c_str() , true );

      ds->Add(node);
      set->push_back( node );
      ++it;
  }

  return set;
}

bool mitk::PersistenceService::RestorePropertyListsFromPersistentDataNodes( const DataStorage* storage )
{
  this->Initialize();
  bool oneFound = false;
  DataStorage::SetOfObjects::ConstPointer allNodes = storage->GetAll();
  for ( mitk::DataStorage::SetOfObjects::const_iterator sourceIter = allNodes->begin();
      sourceIter != allNodes->end();
      ++sourceIter )
  {
      mitk::DataNode* node = *sourceIter;
      bool isPersistenceNode = false;
      //node->GetBoolProperty( PERSISTENCE_PROPERTY_NAME.c_str(), isPersistenceNode ); //see bug 17729
      node->GetBoolProperty( GetPersistencePropertyName().c_str(), isPersistenceNode );

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

bool mitk::PersistenceService::GetAutoLoadAndSave()
{
  this->Initialize();
  return m_AutoLoadAndSave;
}

bool mitk::PersistenceService::RemovePropertyList( std::string& id )
{
  this->Initialize();
  std::map<std::string, mitk::PropertyList::Pointer>::iterator it = m_PropertyLists.find( id );
  if( it != m_PropertyLists.end() )
  {
      m_PropertyLists.erase(it);
      return true;
  }
  return false;
}

void mitk::PersistenceService::Initialize()
{
  if( m_Initialized || m_InInitialized)
    return;
  m_InInitialized = true;

  m_PropertyListsXmlFileReaderAndWriter = PropertyListsXmlFileReaderAndWriter::New();

  // Load Default File in any case
  this->Load();
  //std::string id = mitk::PersistenceService::PERSISTENCE_PROPERTYLIST_NAME; //see bug 17729
  std::string id = GetPersistencePropertyListName();
  mitk::PropertyList::Pointer propList = this->GetPropertyList( id );
  bool autoLoadAndSave = true;
  propList->GetBoolProperty("m_AutoLoadAndSave", autoLoadAndSave);

  if( autoLoadAndSave == false )
  {
    MITK_DEBUG("mitk::PersistenceService") << "autoloading was not wished. clearing data we got so far.";
    this->SetAutoLoadAndSave(false);
    this->Clear();
  }

  m_InInitialized = false;
  m_Initialized = true;
}

void mitk::PersistenceService::Unitialize()
{
  if(this->GetAutoLoadAndSave())
    this->Save();
}