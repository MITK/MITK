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
#include <Poco/Zip/ZipManipulator.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/Thread.h>

#include "mitkSceneIO.h"
#include "mitkBaseDataSerializer.h"
#include "mitkPropertyListSerializer.h"
#include "mitkSceneReader.h"

#include "mitkProgressBar.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkStandaloneDataStorage.h"
#include <mitkStandardFileLocations.h>

#include <itkObjectFactoryBase.h>

#include <tinyxml.h>

#include <fstream>
#include <sstream>
#include <mitkIOUtil.h>

#include "itksys/SystemTools.hxx"

namespace mitk {

class CompressorTask : public Poco::Runnable {
public:
    CompressorTask(mitk::SceneIO::Pointer sceneIO, const std::string& filename, bool incrementalSave, const std::set<std::string>& filesToMaintain)
        : sceneIO(sceneIO)
        , filename(filename)
        , incrementalSave(incrementalSave)
        , filesToMaintain(filesToMaintain)
    {

    }

    virtual void run()
    {
        try
        {
            if (incrementalSave) {
                MITK_INFO << "Saving incrementally";
                Poco::Zip::ZipManipulator zipper(filename, true);

                for (auto iter = zipper.originalArchive().fileInfoBegin(); iter != zipper.originalArchive().fileInfoEnd(); ++iter) {
                    if (filesToMaintain.find(iter->first) == filesToMaintain.end()) {
                        zipper.deleteFile(iter->first);
                    }
                }

                zipper.commit();

                Poco::Zip::ZipManipulator zipper2(filename, false);

                Poco::DirectoryIterator end;
                for (Poco::DirectoryIterator iter(sceneIO->m_WorkingDirectory); iter != end; ++iter) {
                    if (!iter->isDirectory()) {
                        zipper2.addFile(Poco::Path(iter->path()).getFileName(), iter->path(), Poco::Zip::ZipCommon::CM_DEFLATE, Poco::Zip::ZipCommon::CL_SUPERFAST);
                    }
                }

                zipper2.commit();
            }
            else {
                MITK_INFO << "Performing full save";
                Poco::File deleteFile(filename.c_str());
                if (deleteFile.exists())
                {
                    deleteFile.remove();
                }

                // create zip at filename
                std::ofstream file(filename.c_str(), std::ios::binary | std::ios::out);
                if (!file.good())
                {
                    MITK_ERROR << "Could not open a zip file for writing: '" << filename << "'";
                    return;
                }
                else
                {
                    Poco::Zip::Compress zipper(file, true);
                    Poco::Path tmpdir(sceneIO->m_WorkingDirectory);
                    zipper.addRecursive(tmpdir, Poco::Zip::ZipCommon::CL_SUPERFAST);
                    zipper.close();
                }
            }
            ProgressBar::GetInstance()->Progress();

            try
            {
                Poco::File deleteDir(sceneIO->m_WorkingDirectory);
                deleteDir.remove(true); // recursive
            }
            catch (...)
            {
                MITK_ERROR << "Could not delete temporary directory " << sceneIO->m_WorkingDirectory;
                return; // ok?
            }

        }
        catch (std::exception& e)
        {
            ProgressBar::GetInstance()->Progress();
            MITK_ERROR << "Could not create ZIP file from " << sceneIO->m_WorkingDirectory << "\nReason: " << e.what();
            return;
        }
        sceneIO->m_LoadedProjectFileName = filename;
        sceneIO->m_FileTimeStamp = Poco::File(filename).getLastModified();
    }

    mitk::SceneIO* sceneIO;
    std::string filename;
    bool incrementalSave;
    const std::set<std::string> filesToMaintain;
};

}

mitk::SceneIO::SceneIO()
  :m_WorkingDirectory(""),
  m_UnzipErrors(0)
{
}

mitk::SceneIO::~SceneIO()
{
    if (m_CompressionThread.isRunning()) {
        MITK_INFO << "Waiting for save thread to complete...";
        m_CompressionThread.join();
    }
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

mitk::DataStorage::Pointer mitk::SceneIO::LoadScene( const std::string& filename,
                                                    DataStorage* pStorage,
                                                    bool clearStorageFirst )
{
    if (!m_LoadedProjectFileName.empty()) {
        Clear();
    }

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

  m_FileTimeStamp = Poco::File(filename).getLastModified();

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
  TiXmlDocument document( m_WorkingDirectory + mitk::IOUtil::GetDirectorySeparator() + "index.xml" );
  if (!document.LoadFile())
  {
    MITK_ERROR << "Could not open/read/parse " << m_WorkingDirectory << mitk::IOUtil::GetDirectorySeparator() << "index.xml\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return storage;
  }

  TiXmlElement* versionObject = document.FirstChildElement("Version");
  bool incrementalEnabledScene = versionObject && versionObject->Attribute("IncrementalEnabled");

  if (incrementalEnabledScene) {
      storage->AddNodeEvent.AddListener(mitk::MessageDelegate1<SceneIO, const mitk::DataNode*>(this, &SceneIO::RecordLoadTimeStamp));
  }
  SceneReader::Pointer reader = SceneReader::New();
  if ( !reader->LoadScene( document, m_WorkingDirectory, storage, &m_LoadedNodeFileNames ) )
  {
    MITK_ERROR << "There were errors while loading scene file " << filename << ". Your data may be corrupted";
  }
  if (incrementalEnabledScene) {
      storage->AddNodeEvent.RemoveListener(mitk::MessageDelegate1<SceneIO, const mitk::DataNode*>(this, &SceneIO::RecordLoadTimeStamp));
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

  m_LoadedProjectFileName = filename;

  // return new data storage, even if empty or uncomplete (return as much as possible but notify calling method)
  return storage;
}

void mitk::SceneIO::RecordLoadTimeStamp(const mitk::DataNode* node)
{                      
    MITK_INFO << "Loaded " << node->GetName();
    m_NodeLoadTimeStamps[node] = node->GetMTime();
}

bool mitk::SceneIO::SaveScene( DataStorage::SetOfObjects::ConstPointer sceneNodes, const DataStorage* storage,
                              const std::string& filename)
{
    if (m_CompressionThread.isRunning()) {
        MITK_INFO << "Waiting for save thread to complete...";
        m_CompressionThread.join();
    }

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
    version->SetAttribute("IncrementalEnabled", 1);
    document.LinkEndChild(version);

    //DataStorage::SetOfObjects::ConstPointer sceneNodes = storage->GetSubset( predicate );

    Poco::File fileInfo(filename);
    bool incrementalSave = fileInfo.exists() && fileInfo.path() == Poco::File(m_LoadedProjectFileName).path() && m_FileTimeStamp == fileInfo.getLastModified();

    std::set<std::string> filesToMaintain;
    if (sceneNodes.IsNull())
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

      ProgressBar::GetInstance()->AddStepsToDo( sceneNodes->size() + 1 ); // 1 is for compressor thread

      // find out about dependencies
      typedef std::map< DataNode*, std::string > UIDMapType;
      typedef std::map< DataNode*, std::list<std::string> > SourcesMapType;

      UIDMapType nodeUIDs;              // for dependencies: ID of each node
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

      // write out objects, dependencies and properties
      for (DataStorage::SetOfObjects::const_iterator iter = sceneNodes->begin();
        iter != sceneNodes->end();
        ++iter)
      {
        DataNode* node = iter->GetPointer();

        if (node)
        {
          TiXmlElement* nodeElement = new TiXmlElement("node");
          std::string filenameHint( node->GetName() );
          filenameHint = itksys::SystemTools::MakeCindentifier(filenameHint.c_str()); // escape filename <-- only allow [A-Za-z0-9_], replace everything else with _
          filenameHint.resize(std::min(filenameHint.size(), static_cast<size_t>(100))); // Limit file name to support windows file systems with limit of 255 symbols

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
          BaseData* data = node->GetData();
          if (data)
          {
              bool dataNeedsToBeWritten = (!incrementalSave || m_NodeLoadTimeStamps.find(node) == m_NodeLoadTimeStamps.end() || node->GetData()->GetMTime() > m_NodeLoadTimeStamps[node]);
              //std::string filenameHint( node->GetName() );
            bool error(false);
            TiXmlElement* dataElement;
            
            if (dataNeedsToBeWritten) {
                dataElement = SaveBaseData(data, filenameHint, error); // returns a reference to a file
                if (error)
                {
                    m_FailedNodes->push_back(node);
                }
                else {
                    m_LoadedNodeFileNames[node].baseDataFiles.clear();
                    m_LoadedNodeFileNames[node].baseDataFiles.push_back(dataElement->Attribute("file"));

                    for (TiXmlElement* element = dataElement->FirstChildElement("additionalFile"); element != NULL; element = element->NextSiblingElement("additionalFile"))
                    {
                        m_LoadedNodeFileNames[node].baseDataFiles.push_back(element->Attribute("file"));
                    }

                    m_NodeLoadTimeStamps[node] = node->GetMTime();
                }
                MITK_INFO << "Saved " << node->GetName();
            }
            else {
                // Create the element using stored data
                dataElement = new TiXmlElement("data");
                dataElement->SetAttribute("type", data->GetNameOfClass());
                dataElement->SetAttribute("file", m_LoadedNodeFileNames[node].baseDataFiles[0]);
                for (size_t i = 1; i < m_LoadedNodeFileNames[node].baseDataFiles.size(); ++i)
                {
                    TiXmlElement* additionalFileElement = new TiXmlElement("additionalFile");
                    additionalFileElement->SetAttribute("file", m_LoadedNodeFileNames[node].baseDataFiles[i]);
                    dataElement->LinkEndChild(additionalFileElement);
                }

                std::copy(m_LoadedNodeFileNames[node].baseDataFiles.begin(), m_LoadedNodeFileNames[node].baseDataFiles.end(), std::inserter(filesToMaintain, filesToMaintain.end()));
                MITK_INFO << "Skipped " << node->GetName();
            }

            // store basedata properties
            PropertyList* propertyList = data->GetPropertyList();
            if (propertyList && !propertyList->IsEmpty() )
            {
              TiXmlElement* baseDataPropertiesElement( SavePropertyList( propertyList, filenameHint + "-data") ); // returns a reference to a file
              m_LoadedNodeFileNames[node].baseDataPropertiesFile = baseDataPropertiesElement->Attribute("file");
              dataElement->LinkEndChild( baseDataPropertiesElement );
            }

            nodeElement->LinkEndChild( dataElement );
          }

          // store all renderwindow specific propertylists
          m_LoadedNodeFileNames[node].nodePropertiesFiles.clear();
          if (RenderingManager::IsInstantiated())
          {
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
                  
                  m_LoadedNodeFileNames[node].nodePropertiesFiles[renderWindowName] = renderWindowPropertiesElement->Attribute("file");
                  
                  renderWindowPropertiesElement->SetAttribute("renderwindow", renderWindowName);
                  nodeElement->LinkEndChild( renderWindowPropertiesElement );
                }
              }
            }
          }

          // don't forget the renderwindow independent list
          PropertyList* propertyList = node->GetPropertyList();
          if ( propertyList && !propertyList->IsEmpty() )
          {
            TiXmlElement* propertiesElement( SavePropertyList( propertyList, filenameHint + "-node") ); // returns a reference to a file

            m_LoadedNodeFileNames[node].nodePropertiesFiles[""] = propertiesElement->Attribute("file");

            nodeElement->LinkEndChild(propertiesElement);
          }
          document.LinkEndChild( nodeElement );
        }
        else
        {
          MITK_WARN << "Ignoring NULL node during scene serialization.";
        }

        ProgressBar::GetInstance()->Progress();
      } // end for all nodes
    } // end if sceneNodes

    if ( !document.SaveFile( m_WorkingDirectory + Poco::Path::separator() + "index.xml" ) )
    {
      MITK_ERROR << "Could not write scene to " << m_WorkingDirectory << Poco::Path::separator() << "index.xml" << "\nTinyXML reports '" << document.ErrorDesc() << "'";
      return false;
    }
    else
    {
        m_CompressorTask.reset(new CompressorTask(this, filename, incrementalSave, filesToMaintain));
        m_CompressionThread.start(*m_CompressorTask);
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
      serializer->SetWorkingDirectory( m_WorkingDirectory );
      try
      {
        std::vector<std::string> writtenfilenames = serializer->SerializeAll();
        if (!writtenfilenames.empty()) {
            element->SetAttribute("file", writtenfilenames[0]);
            for (size_t i = 1; i < writtenfilenames.size(); ++i) {
                TiXmlElement* additionalFileElement = new TiXmlElement("additionalFile");
                additionalFileElement->SetAttribute("file", writtenfilenames[i]);
                element->LinkEndChild(additionalFileElement);
            }
            error = false;
        }
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

void mitk::SceneIO::Clear()
{
    m_LoadedProjectFileName.clear();
    m_NodeLoadTimeStamps.clear();
    m_LoadedNodeFileNames.clear();
}
