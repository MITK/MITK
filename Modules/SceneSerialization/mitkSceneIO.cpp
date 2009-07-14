/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <Poco/TemporaryFile.h>
#include <Poco/Path.h>
#include <Poco/Zip/Compress.h>

#include "mitkSceneIO.h"
#include "mitkBaseDataSerializer.h"
#include "mitkPropertyListSerializer.h"

#include "mitkProgressBar.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include <itkObjectFactoryBase.h>

#include <tinyxml.h>

#include <fstream>
    
mitk::SceneIO::SceneIO()
:m_WorkingDirectory("")
{
}

mitk::SceneIO::~SceneIO()
{
}

std::string mitk::SceneIO::CreateEmptyTempDirectory()
{
  std::string uniquename = Poco::TemporaryFile::tempName();
  Poco::File tempdir( uniquename ); 
  try
  {
    if (!tempdir.createDirectory())
    {
      LOG_ERROR << "Could not create temporary directory " << uniquename;
      return "";
    }
  }
  catch( std::exception& e )
  {
      LOG_ERROR << "Could not create temporary directory " << uniquename << ":" << e.what();
      return "";
  }
      
  return uniquename;
}

mitk::DataStorage::Pointer mitk::SceneIO::LoadScene( const std::string& filename, 
                                                     DataStorage* storage, 
                                                     bool clearStorageFirst )
{
  return NULL;
}
    
bool mitk::SceneIO::SaveScene( DataStorage* storage,
                               const std::string& filename,
                               NodePredicateBase* predicate )
{
  m_FailedNodes = DataStorage::SetOfObjects::New();
  m_FailedProperties = PropertyList::New();

  if (!storage) 
  {
    LOG_ERROR << "No data storage given. Not possible to save scene.";
    return false;
  }

  if ( filename.empty() )
  {
    LOG_ERROR << "No filename given. Not possible to save scene.";
    return false;
  }

  // start XML DOM
  TiXmlDocument document;
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" ); // TODO what to write here? encoding? etc....
  document.LinkEndChild( decl );

  TiXmlElement* version = new TiXmlElement("Version");
  version->SetAttribute("Writer",  __FILE__ );
  version->SetAttribute("Revision",  "$Revision: 17055 $" );
  version->SetAttribute("FileVersion",  1 );
  document.LinkEndChild(version);
 
  DataStorage::SetOfObjects::ConstPointer storedObjects = storage->GetSubset( predicate );

  if ( storedObjects.IsNull() )
  {
    LOG_WARN << "Saving empty scene to " << filename;
  }
  else
  {
    if ( storedObjects->size() == 0 )
    {
      LOG_WARN << "Saving empty scene to " << filename;
    }

    LOG_INFO << "Storing scene with " << storedObjects->size() << " objects to " << filename;
  
    m_WorkingDirectory = CreateEmptyTempDirectory();
    if (m_WorkingDirectory.empty())
    {
      LOG_ERROR << "Could not create temporary directory. Cannot create scene files.";
      return false;
    }
   
    ProgressBar::GetInstance()->AddStepsToDo( storedObjects->size() );
 
    // find out about dependencies
    typedef std::map< DataTreeNode*, std::string > UIDMapType;
    typedef std::map< DataTreeNode*, std::list<std::string> > SourcesMapType;

    UIDMapType nodeUIDs;              // for dependencies: ID of each node
    SourcesMapType sourceUIDs; // for dependencies: IDs of a node's parent nodes

    UIDGenerator nodeUIDGen("OBJECT_");

    for (DataStorage::SetOfObjects::const_iterator iter = storedObjects->begin();
         iter != storedObjects->end();
         ++iter)
    {
      DataTreeNode* node = iter->GetPointer();
      if (!node)  continue; // unlikely event that we get a NULL pointer as an object for saving. just ignore
    
      // generate UIDs for all source objects  
      DataStorage::SetOfObjects::ConstPointer sourceObjects = storage->GetSources( node );
      for ( mitk::DataStorage::SetOfObjects::const_iterator sourceIter = sourceObjects->begin();
            sourceIter != sourceObjects->end();
            ++sourceIter )
      {
        if ( std::find( storedObjects->begin(), storedObjects->end(), *sourceIter ) == storedObjects->end() ) continue; // source is not saved, so don't generate a UID for this source

        // create a uid for the parent object
        if ( nodeUIDs[ *sourceIter ].empty() )
        {
          nodeUIDs[ *sourceIter ] = nodeUIDGen.GetUID();
        }

        // store this dependency for writing 
        sourceUIDs[ node ].push_back( nodeUIDs[*sourceIter] );
      }
    }
   
    // write out objects, dependencies and properties
    for (DataStorage::SetOfObjects::const_iterator iter = storedObjects->begin();
         iter != storedObjects->end();
         ++iter)
    {
      DataTreeNode* node = iter->GetPointer();
      
      if (node)
      {
        TiXmlElement* nodeElement = new TiXmlElement("node");
        std::string filenameHint( node->GetName() );
          
        // store dependencies
        UIDMapType::iterator searchUIDIter = nodeUIDs.find(node);
        if ( searchUIDIter != nodeUIDs.end() )
        {
          // store this node's ID
          nodeElement->SetAttribute("UID", searchUIDIter->second.c_str() );
        }
        
        SourcesMapType::iterator searchSourcesIter = sourceUIDs.find(node);
        if ( searchSourcesIter != sourceUIDs.end() )
        {
          // store all source IDs
          for ( std::list<std::string>::iterator sourceUIDIter = searchSourcesIter->second.begin();
                sourceUIDIter != searchSourcesIter->second.end();
                ++sourceUIDIter )
          {
            TiXmlElement* uidElement = new TiXmlElement("source");
            uidElement->SetAttribute("UID", sourceUIDIter->c_str() );
            nodeElement->LinkEndChild( uidElement );
          }
        }

        // store basedata
        if ( BaseData* data = node->GetData() )
        {
          std::string filenameHint( node->GetName() );
          bool error(false);
          TiXmlElement* dataElement( SaveBaseData( data, filenameHint, error ) ); // returns a reference to a file
          if (error)
          {
            m_FailedNodes->push_back( node );
          }

          // store basedata properties
          PropertyList* propertyList = data->GetPropertyList();
          if (propertyList && !propertyList->IsEmpty() )
          {
            TiXmlElement* baseDataPropertiesElement( SavePropertyList( propertyList, filenameHint + "-data") ); // returns a reference to a file
            dataElement->LinkEndChild( baseDataPropertiesElement );
          }

          nodeElement->LinkEndChild( dataElement );
        }

        // store all renderwindow specific propertylists
        const RenderingManager::RenderWindowVector& allRenderWindows( RenderingManager::GetInstance()->GetAllRegisteredRenderWindows() );
        for ( RenderingManager::RenderWindowVector::const_iterator rw = allRenderWindows.begin();
              rw != allRenderWindows.end();
              ++rw)
        {
          if (vtkRenderWindow* renderWindow = *rw)
          {
            std::string renderWindowName( mitk::BaseRenderer::GetInstance(renderWindow)->GetName() );
            BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderWindow);
            PropertyList* propertyList = node->GetPropertyList(renderer);
            if ( propertyList && !propertyList->IsEmpty() )
            {
              TiXmlElement* renderWindowPropertiesElement( SavePropertyList( propertyList, filenameHint + "-" + renderWindowName) ); // returns a reference to a file
              renderWindowPropertiesElement->SetAttribute("render window", renderWindowName);
              nodeElement->LinkEndChild( renderWindowPropertiesElement );
            }
          }
        }

        // don't forget the renderwindow independent list
        PropertyList* propertyList = node->GetPropertyList();
        if ( propertyList && !propertyList->IsEmpty() )
        {
          TiXmlElement* propertiesElement( SavePropertyList( propertyList, filenameHint + "-node") ); // returns a reference to a file
          nodeElement->LinkEndChild( propertiesElement );
        }

            
        document.LinkEndChild( nodeElement );
      }
      else
      {
        LOG_WARN << "Ignoring NULL node during scene serialization.";
      }
        
      ProgressBar::GetInstance()->Progress();
    } // end for all nodes

  } // end if storedObjects

  if ( !document.SaveFile( m_WorkingDirectory + "/index.xml" ) )
  {
    LOG_ERROR << "Could not write scene to " << filename << "\nTinyXML reports '" << document.ErrorDesc() << "'";
    return false;
  }
  else
  {
    try
    {
      // create zip at filename
      std::ofstream file( filename.c_str(), std::ios::binary | std::ios::out);
      Poco::Zip::Compress zipper( file, true );
      Poco::Path tmpdir( m_WorkingDirectory );
      zipper.addRecursive( tmpdir );
      zipper.close();
      Poco::File deleteDir( m_WorkingDirectory );
      deleteDir.remove(true); // recursive
    }
    catch(std::exception& /*e*/)
    {
      LOG_ERROR << "Could not create ZIP file from " << m_WorkingDirectory;
      return false;
    }

    return true;
  }
}

TiXmlElement* mitk::SceneIO::SaveBaseData( BaseData* data, const std::string& filenamehint, bool& error )
{
  assert(data);
  error = true;

  // find correct serializer
  // the serilizer must
  //  - create a file containing all information to recreate the BaseData object --> needs to know where to put this file (and a filename?)
  //  - TODO what to do about writers that creates one file per timestep?
  TiXmlElement* element = new TiXmlElement("data");
  element->SetAttribute( "type", data->GetNameOfClass() );
 
  // construct name of serializer class
  std::string serializername(data->GetNameOfClass());
  serializername += "Serializer";
  
  std::list<itk::LightObject::Pointer> thingsThatCanSerializeThis = itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (thingsThatCanSerializeThis.size() < 1)
  {
    LOG_ERROR << "No serializer found for " << data->GetNameOfClass() << ". Skipping object";
  }
  if (thingsThatCanSerializeThis.size() > 1)
  {
    LOG_ERROR << "Multiple serializers found for " << data->GetNameOfClass() << "Using arbitrary first one.";
  }

  for ( std::list<itk::LightObject::Pointer>::iterator iter = thingsThatCanSerializeThis.begin();
        iter != thingsThatCanSerializeThis.end();
        ++iter )
  {
    if (BaseDataSerializer* serializer = dynamic_cast<BaseDataSerializer*>( iter->GetPointer() ) )
    {
      serializer->SetData(data);
      serializer->SetFilenameHint(filenamehint);
      serializer->SetWorkingDirectory( m_WorkingDirectory );
      try
      {
        std::string writtenfilename = serializer->Serialize();
        element->SetAttribute("file", writtenfilename);
        error = false;
      }
      catch (std::exception& e)
      {
        LOG_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
      }
      break;
    }
  }

  return element;
}

TiXmlElement* mitk::SceneIO::SavePropertyList( PropertyList* propertyList, const std::string& filenamehint)
{
  assert(propertyList);
  
  //  - TODO what to do about shared properties (same object in two lists or behind several keys)?
  TiXmlElement* element = new TiXmlElement("properties");
 
  // construct name of serializer class
  PropertyListSerializer::Pointer serializer = PropertyListSerializer::New();
  
  serializer->SetPropertyList(propertyList);
  serializer->SetFilenameHint(filenamehint);
  serializer->SetWorkingDirectory( m_WorkingDirectory );
  try
  {
    std::string writtenfilename = serializer->Serialize();
    element->SetAttribute("file", writtenfilename);
    PropertyList::Pointer failedProperties = serializer->GetFailedProperties();
    if (failedProperties.IsNotNull())
    {
      // move failed properties to global list
      m_FailedProperties->ConcatenatePropertyList( failedProperties, true );
    }
  }
  catch (std::exception& e)
  {
    LOG_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
  }
    
  return element;
}

    
const mitk::SceneIO::FailedBaseDataListType* mitk::SceneIO::GetFailedNodes() 
{ 
  return m_FailedNodes.GetPointer(); 
}
    
const mitk::PropertyList* mitk::SceneIO::GetFailedProperties() 
{ 
  return m_FailedProperties; 
}
