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

#include <Poco/TemporaryFile.h>
#include <Poco/Path.h>
#include <Poco/Delegate.h>
#include <Poco/Zip/Compress.h>
#include <Poco/Zip/Decompress.h>

#include "mitkSceneIO.h"
#include "mitkBaseDataSerializer.h"
#include "mitkPropertyListSerializer.h"
#include "mitkSceneReader.h"

#include "mitkProgressBar.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkStandaloneDataStorage.h"
#include <mitkStandardFileLocations.h>
#include <mitkLocaleSwitch.h>

#include <itkObjectFactoryBase.h>

#include <tinyxml.h>

#include <fstream>
#include <sstream>
#include <mitkIOUtil.h>

#include "itksys/SystemTools.hxx"

#include "AutoplanLogging.h"

mitk::SceneIO::SceneIO()
  :m_WorkingDirectory(""),
  m_UnzipErrors(0)
{
}

mitk::SceneIO::~SceneIO()
{
}

std::string mitk::SceneIO::CreateEmptyTempDirectory()
{
  mitk::UIDGenerator uidGen("UID_",6);

  //std::string returnValue = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + Poco::Path::separator() + "SceneIOTemp" + uidGen.GetUID();
  std::string returnValue = Poco::Path::temp() + "SceneIOTemp" + uidGen.GetUID();
  std::string uniquename = returnValue + Poco::Path::separator();
  Poco::File tempdir( uniquename );

  try
  {
    bool existsNot = tempdir.createDirectory();
    if (!existsNot)
    {
      MITK_ERROR << "Warning: Directory already exitsts: " << uniquename << " (choosing another)";
      returnValue = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + Poco::Path::separator() + "SceneIOTempDirectory" + uidGen.GetUID();
      uniquename = returnValue + Poco::Path::separator();
      Poco::File tempdir2( uniquename );
      if (!tempdir2.createDirectory())
      {
        MITK_ERROR << "Warning: Second directory also already exitsts: " << uniquename;
      }
    }
  }
  catch( std::exception& e )
  {
    MITK_ERROR << "Could not create temporary directory " << uniquename << ":" << e.what();
    return "";
  }

  return returnValue;
}

mitk::DataStorage::Pointer mitk::SceneIO::LoadWorkspace( const std::string& workDir,
                                                     DataStorage* pStorage,
                                                     bool clearStorageFirst )
{

  // prepare data storage
  DataStorage::Pointer storage = pStorage;
  if ( storage.IsNull() )
  {
    storage = StandaloneDataStorage::New().GetPointer();
  }

  if ( clearStorageFirst )
  {
    try
    {
      storage->Remove( storage->GetAll() );
    }
    catch(...)
    {
      MITK_ERROR << "DataStorage cannot be cleared properly.";
    }
  }

  // test input filename
  if ( workDir.empty() )
  {
    MITK_ERROR << "No workDir given. Not possible to load scene.";
    return storage;
  }

  // test if index.xml exists
  // parse index.xml with TinyXML
  TiXmlDocument document( workDir + Poco::Path::separator() + "index.xml" );
  if (!document.LoadFile())
  {
    MITK_ERROR << "Could not open/read/parse " << workDir << "/index.xml\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return storage;
  }

  SceneReader::Pointer reader = SceneReader::New();
  if ( !reader->LoadScene( document, workDir, storage ) )
  {
    MITK_ERROR << "There were errors while loading scene file in " << workDir << ". Your data may be corrupted";
  }

  // return new data storage, even if empty or uncomplete (return as much as possible but notify calling method)
  return storage;

}

mitk::DataStorage::Pointer mitk::SceneIO::LoadScene( const std::string& filename,
                                                    DataStorage* pStorage,
                                                    bool clearStorageFirst )
{
  mitk::LocaleSwitch localeSwitch("C");

  AUTOPLAN_INFO << "Loading MITK file: " << filename;

  // prepare data storage
  DataStorage::Pointer storage = pStorage;
  if ( storage.IsNull() )
  {
    storage = StandaloneDataStorage::New().GetPointer();
  }

  if ( clearStorageFirst )
  {
    try
    {
      storage->Remove( storage->GetAll() );
    }
    catch(...)
    {
      MITK_ERROR << "DataStorage cannot be cleared properly.";
    }
  }

  // test input filename
  if ( filename.empty() )
  {
    MITK_ERROR << "No filename given. Not possible to load scene.";
    return storage;
  }

  // test if filename can be read
  std::ifstream file( filename.c_str(), std::ios::binary );
  if (!file.good())
  {
    MITK_ERROR << "Cannot open '" << filename << "' for reading";
    return storage;
  }

  // get new temporary directory
  m_WorkingDirectory = CreateEmptyTempDirectory();
  if (m_WorkingDirectory.empty())
  {
    MITK_ERROR << "Could not create temporary directory. Cannot open scene files.";
    return storage;
  }

  // unzip all filenames contents to temp dir
  m_UnzipErrors = 0;
  Poco::Zip::Decompress unzipper( file, Poco::Path( m_WorkingDirectory ) );
  unzipper.EError += Poco::Delegate<SceneIO, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(this, &SceneIO::OnUnzipError);
  unzipper.EOk    += Poco::Delegate<SceneIO, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(this, &SceneIO::OnUnzipOk);
  unzipper.decompressAllFiles();
  unzipper.EError -= Poco::Delegate<SceneIO, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> >(this, &SceneIO::OnUnzipError);
  unzipper.EOk    -= Poco::Delegate<SceneIO, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> >(this, &SceneIO::OnUnzipOk);

  if ( m_UnzipErrors )
  {
    MITK_ERROR << "There were " << m_UnzipErrors << " errors unzipping '" << filename << "'. Will attempt to read whatever could be unzipped.";
  }

  // test if index.xml exists
  // parse index.xml with TinyXML
  std::string defaultLocale_WorkingDirectory = Poco::Path::transcode ( m_WorkingDirectory );
  TiXmlDocument document( defaultLocale_WorkingDirectory + mitk::IOUtil::GetDirectorySeparator() + "index.xml" );
  if (!document.LoadFile())
  {
    MITK_ERROR << "Could not open/read/parse " << defaultLocale_WorkingDirectory << mitk::IOUtil::GetDirectorySeparator() << "index.xml\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return storage;
  }

  SceneReader::Pointer reader = SceneReader::New();
  if ( !reader->LoadScene( document, defaultLocale_WorkingDirectory, storage ) )
  {
    MITK_ERROR << "There were errors while loading scene file " << filename << ". Your data may be corrupted";
  }

  // delete temp directory
  try
  {
    Poco::File deleteDir( m_WorkingDirectory );
    deleteDir.remove(true); // recursive
  }
  catch(...)
  {
    MITK_ERROR << "Could not delete temporary directory " << m_WorkingDirectory;
  }

  // return new data storage, even if empty or uncomplete (return as much as possible but notify calling method)
  return storage;
}

bool mitk::SceneIO::SaveScene( DataStorage::SetOfObjects::ConstPointer sceneNodes, const DataStorage* storage,
                              const std::string& filename)
{
  AUTOPLAN_INFO << "Saving MITK file: " << filename;
  if (!sceneNodes)
  {
    MITK_ERROR << "No set of nodes given. Not possible to save scene.";
    return false;
  }
  if (!storage)
  {
    MITK_ERROR << "No data storage given. Not possible to save scene.";  // \TODO: Technically, it would be possible to save the nodes without their relation
    return false;
  }

  if ( filename.empty() )
  {
    MITK_ERROR << "No filename given. Not possible to save scene.";
    return false;
  }

  mitk::LocaleSwitch localeSwitch("C");

  try
  {
    m_FailedNodes = DataStorage::SetOfObjects::New();
    m_FailedProperties = PropertyList::New();

    // start XML DOM
    TiXmlDocument document;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" ); // TODO what to write here? encoding? standalone would mean that we provide a DTD somewhere...
    document.LinkEndChild( decl );

    TiXmlElement* version = new TiXmlElement("Version");
    version->SetAttribute("Writer",  __FILE__ );
    version->SetAttribute("Revision",  "$Revision: 17055 $" );
    version->SetAttribute("FileVersion",  1 );
    document.LinkEndChild(version);

    //DataStorage::SetOfObjects::ConstPointer sceneNodes = storage->GetSubset( predicate );

    if ( sceneNodes.IsNull() )
    {
      MITK_WARN << "Saving empty scene to " << filename;
    }
    else
    {
      if ( sceneNodes->size() == 0 )
      {
        MITK_WARN << "Saving empty scene to " << filename;
      }

      MITK_INFO << "Storing scene with " << sceneNodes->size() << " objects to " << filename;

      m_WorkingDirectory = CreateEmptyTempDirectory();
      if (m_WorkingDirectory.empty())
      {
        MITK_ERROR << "Could not create temporary directory. Cannot create scene files.";
        return false;
      }

      ProgressBar::GetInstance()->AddStepsToDo( sceneNodes->size() );

      // find out about dependencies
      typedef std::map< DataNode*, std::string > UIDMapType;
      typedef std::map< DataNode*, std::list<std::string> > SourcesMapType;

      UIDMapType nodeUIDs;       // for dependencies: ID of each node
      SourcesMapType sourceUIDs; // for dependencies: IDs of a node's parent nodes

      UIDGenerator nodeUIDGen("OBJECT_");

      for (DataStorage::SetOfObjects::const_iterator iter = sceneNodes->begin();
        iter != sceneNodes->end();
        ++iter)
      {
        DataNode* node = iter->GetPointer();
        if (!node)
          continue; // unlikely event that we get a NULL pointer as an object for saving. just ignore

        // generate UIDs for all source objects
        DataStorage::SetOfObjects::ConstPointer sourceObjects = storage->GetSources( node );
        for ( mitk::DataStorage::SetOfObjects::const_iterator sourceIter = sourceObjects->begin();
          sourceIter != sourceObjects->end();
          ++sourceIter )
        {
          if ( std::find( sceneNodes->begin(), sceneNodes->end(), *sourceIter ) == sceneNodes->end() )
            continue; // source is not saved, so don't generate a UID for this source

          // create a uid for the parent object
          if ( nodeUIDs[ *sourceIter ].empty() )
          {
            nodeUIDs[ *sourceIter ] = nodeUIDGen.GetUID();
          }

          // store this dependency for writing
          sourceUIDs[ node ].push_back( nodeUIDs[*sourceIter] );
        }

        if ( nodeUIDs[ node ].empty() )
        {
          nodeUIDs[ node ] = nodeUIDGen.GetUID();
        }
      }

      bool logNodePresent = false;
      itk::SmartPointer<DataNode> logNode;
      std::string logData;
      if (Logger::Options::get().datastoragelog && Logger::Log::get().getDataBackend())
      {
        std::string log;

        for (auto node : sceneNodes->CastToSTLConstContainer())
        {
          if (node->GetName() == "AutoplanLog") {
            logNode = node;
            logNodePresent = true;
            auto preopertyList = node->GetPropertyList();
            preopertyList->GetStringProperty("log", log);
            if (!log.empty())
            {
              auto lastDateTime = Logger::Log::getLastDateTime(log);
              logData = Logger::Log::get().getDataFromDate(lastDateTime);
            }
            else {
              logData = Logger::Log::get().getData();
            }
            break;
          }
        }
        if (!logNodePresent)
        {
          logNode = DataNode::New();
          logNode->SetName("AutoplanLog");
          logData = Logger::Log::get().getData();
        }

        StringProperty::Pointer myLog =
          StringProperty::New(log + logData.c_str());

        logNode->SetProperty("log", myLog);
      }

      auto serialize = [&](DataNode::Pointer node)
      {
        if (node)
        {
          TiXmlElement* nodeElement = new TiXmlElement("node");
          std::string filenameHint(node->GetName());
          filenameHint = itksys::SystemTools::MakeCindentifier(filenameHint.c_str()); // escape filename <-- only allow [A-Za-z0-9_], replace everything else with _

          // store dependencies
          UIDMapType::iterator searchUIDIter = nodeUIDs.find(node);
          if (searchUIDIter != nodeUIDs.end())
          {
            // store this node's ID
            nodeElement->SetAttribute("UID", searchUIDIter->second.c_str());
          }

          SourcesMapType::iterator searchSourcesIter = sourceUIDs.find(node);
          if (searchSourcesIter != sourceUIDs.end())
          {
            // store all source IDs
            for (std::list<std::string>::iterator sourceUIDIter = searchSourcesIter->second.begin();
              sourceUIDIter != searchSourcesIter->second.end();
              ++sourceUIDIter)
            {
              TiXmlElement* uidElement = new TiXmlElement("source");
              uidElement->SetAttribute("UID", sourceUIDIter->c_str());
              nodeElement->LinkEndChild(uidElement);
            }
          }

          // store basedata
          if (BaseData* data = node->GetData())
          {
            //std::string filenameHint( node->GetName() );
            bool error(false);
            TiXmlElement* dataElement(SaveBaseData(data, filenameHint, error)); // returns a reference to a file
            if (error)
            {
              m_FailedNodes->push_back(node);
            }
            
            if (dataElement == nullptr)
            {
              return;
            }

            // store basedata properties
            PropertyList* propertyList = data->GetPropertyList();
            if (propertyList && !propertyList->IsEmpty())
            {
              TiXmlElement* baseDataPropertiesElement(SavePropertyList(propertyList, filenameHint + "-data")); // returns a reference to a file
              dataElement->LinkEndChild(baseDataPropertiesElement);
            }

            nodeElement->LinkEndChild(dataElement);
          }

          // store all renderwindow specific propertylists
          mitk::DataNode::PropertyListKeyNames propertyListKeys = node->GetPropertyListNames();
          for (auto renderWindowName : propertyListKeys)
          {
            PropertyList* propertyList = node->GetPropertyList(renderWindowName);
            if ( propertyList && !propertyList->IsEmpty() )
            {
              TiXmlElement* renderWindowPropertiesElement( SavePropertyList( propertyList, filenameHint + "-" + renderWindowName) ); // returns a reference to a file
              renderWindowPropertiesElement->SetAttribute("renderwindow", renderWindowName);
              nodeElement->LinkEndChild( renderWindowPropertiesElement );
            }
          }

          // don't forget the renderwindow independent list
          PropertyList* propertyList = node->GetPropertyList();
          if (propertyList && !propertyList->IsEmpty())
          {
            TiXmlElement* propertiesElement(SavePropertyList(propertyList, filenameHint + "-node")); // returns a reference to a file
            nodeElement->LinkEndChild(propertiesElement);
          }
          document.LinkEndChild(nodeElement);
        }
        else
        {
          MITK_WARN << "Ignoring NULL node during scene serialization.";
        }

        ProgressBar::GetInstance()->Progress();
      };

      // write out objects, dependencies and properties
      for (auto node : sceneNodes->CastToSTLConstContainer())
      {
        serialize(node);
      } // end for all nodes

      if (Logger::Options::get().datastoragelog && Logger::Log::get().getDataBackend())
      {
        if (!logNodePresent)
        {
          serialize(logNode);
        }
      }
    } // end if sceneNodes

    std::string defaultLocale_WorkingDirectory = Poco::Path::transcode( m_WorkingDirectory );
    if ( !document.SaveFile( defaultLocale_WorkingDirectory + Poco::Path::separator() + "index.xml" ) )
    {
      MITK_ERROR << "Could not write scene to " << defaultLocale_WorkingDirectory << Poco::Path::separator() << "index.xml" << "\nTinyXML reports '" << document.ErrorDesc() << "'";
      return false;
    }
    else
    {
      try
      {
        Poco::File deleteFile( filename.c_str() );
        if (deleteFile.exists())
        {
          deleteFile.remove();
        }

        // create zip at filename
        std::string defaultLocaleFilename = Poco::Path::transcode( filename );
        std::ofstream file( defaultLocaleFilename.c_str(), std::ios::binary | std::ios::out);
        if (!file.good())
        {
          MITK_ERROR << "Could not open a zip file for writing: '" << defaultLocaleFilename << "'";
          return false;
        }
        else
        {
          Poco::Zip::Compress zipper( file, true );
          Poco::Path tmpdir( m_WorkingDirectory );
          zipper.addRecursive( tmpdir );
          zipper.close();
        }
        try
        {
          Poco::File deleteDir( m_WorkingDirectory );
          deleteDir.remove(true); // recursive
        }
        catch(...)
        {
          MITK_ERROR << "Could not delete temporary directory " << m_WorkingDirectory;
          return false; // ok?
        }
      }
      catch(std::exception& e)
      {
        MITK_ERROR << "Could not create ZIP file from " << m_WorkingDirectory << "\nReason: " << e.what();
        return false;
      }
      return true;
    }
  }
  catch(std::exception& e)
  {
    MITK_ERROR << "Caught exception during saving temporary files to disk. Error description: '" << e.what() << "'";
    return false;
  }
}

TiXmlElement* mitk::SceneIO::SaveBaseData( BaseData* data, const std::string& filenamehint, bool& error )
{
  assert(data);
  error = true;

  // find correct serializer
  // the serializer must
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
    MITK_ERROR << "No serializer found for " << data->GetNameOfClass() << ". Skipping object";
  }

  for ( std::list<itk::LightObject::Pointer>::iterator iter = thingsThatCanSerializeThis.begin();
    iter != thingsThatCanSerializeThis.end();
    ++iter )
  {
    if (BaseDataSerializer* serializer = dynamic_cast<BaseDataSerializer*>( iter->GetPointer() ) )
    {
      serializer->SetData(data);
      serializer->SetFilenameHint(filenamehint);
      std::string defaultLocale_WorkingDirectory = Poco::Path::transcode( m_WorkingDirectory );
      serializer->SetWorkingDirectory( defaultLocale_WorkingDirectory );
      try
      {
        std::string writtenfilename = serializer->Serialize();
        
        if (writtenfilename.empty())
        {
          return nullptr;
        }
        
        element->SetAttribute("file", writtenfilename);
        error = false;
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
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
  std::string defaultLocale_WorkingDirectory = Poco::Path::transcode( m_WorkingDirectory );
  serializer->SetWorkingDirectory( defaultLocale_WorkingDirectory );
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
    MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
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

void mitk::SceneIO::OnUnzipError(const void*  /*pSender*/, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info)
{
  ++m_UnzipErrors;
  MITK_ERROR << "Error while unzipping: " << info.second;
}

void mitk::SceneIO::OnUnzipOk(const void*  /*pSender*/, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& /*info*/)
{
  // MITK_INFO << "Unzipped ok: " << info.second.toString();
}
