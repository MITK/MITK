/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <Poco/Delegate.h>
#include <Poco/Path.h>
#include <Poco/TemporaryFile.h>
#include <Poco/Zip/Compress.h>
#include <Poco/Zip/Decompress.h>

#include <mitkBaseDataSerializer.h>
#include <mitkPropertyListSerializer.h>
#include <mitkSceneIO.h>
#include <mitkSceneJsonReader.h>
#include <mitkSceneReader.h>

#include <mitkBaseRenderer.h>
#include <mitkProgressBar.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkLocaleSwitch.h>
#include <mitkStandardFileLocations.h>
#include <mitkStringUtil.h>
#include <mitkUIDGenerator.h>

#include <mitkCoreServices.h>
#include <mitkIPropertyTransience.h>

#include <itkObjectFactoryBase.h>

#include <fstream>
#include <mitkIOUtil.h>
#include <sstream>

#include <itksys/SystemTools.hxx>

#include <tinyxml2.h>

mitk::SceneIO::SceneIO() : m_WorkingDirectory(""), m_UnzipErrors(0)
{
}

mitk::SceneIO::~SceneIO()
{
}

std::string mitk::SceneIO::CreateEmptyTempDirectory()
{
  mitk::UIDGenerator uidGen;

  // std::string returnValue = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() +
  // Poco::Path::separator() + "SceneIOTemp" + uidGen.GetUID();
  std::string returnValue = Poco::Path::temp() + "SceneIOTemp" + uidGen.GetUID();
  std::string uniquename = returnValue + Poco::Path::separator();
  Poco::File tempdir(uniquename);

  try
  {
    bool existsNot = tempdir.createDirectory();
    if (!existsNot)
    {
      MITK_ERROR << "Warning: Directory already exists: " << uniquename << " (choosing another)";
      returnValue = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + Poco::Path::separator() +
                    "SceneIOTempDirectory" + uidGen.GetUID();
      uniquename = returnValue + Poco::Path::separator();
      Poco::File tempdir2(uniquename);
      if (!tempdir2.createDirectory())
      {
        MITK_ERROR << "Warning: Second directory also already exists: " << uniquename;
      }
    }
  }
  catch (std::exception &e)
  {
    MITK_ERROR << "Could not create temporary directory " << uniquename << ":" << e.what();
    return "";
  }

  return returnValue;
}

mitk::DataStorage::Pointer mitk::SceneIO::LoadScene(const std::string &filename,
                                                    DataStorage *pStorage,
                                                    bool clearStorageFirst)
{
  mitk::LocaleSwitch localeSwitch("C");

  // prepare data storage
  DataStorage::Pointer storage = pStorage;
  if (storage.IsNull())
  {
    storage = StandaloneDataStorage::New().GetPointer();
  }

  // test input filename
  if (filename.empty())
  {
    MITK_ERROR << "No filename given. Not possible to load scene.";
    return storage;
  }

  // Standalone JSON scene (not a ZIP archive): route directly to the
  // JSON reader without unpacking.
  if (mitk::EndsWithCaseInsensitive(filename, ".mitkscene.json"))
  {
    // Clearing is delegated to the reader so it can be deferred until
    // after the scene descriptor has been validated (a malformed JSON
    // file must not wipe the caller's session).
    try
    {
      SceneJsonReader::Pointer jsonReader = SceneJsonReader::New();
      if (!jsonReader->LoadScene(filename, storage, clearStorageFirst))
      {
        MITK_ERROR << "There were errors while loading scene file " << filename
                   << ". Your data may be corrupted";
      }
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "Failed to load JSON scene file '" << filename << "': " << e.what();
    }

    return storage;
  }

  // test if filename can be read
  std::ifstream file(filename.c_str(), std::ios::binary);
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
  Poco::Zip::Decompress unzipper(file, Poco::Path(m_WorkingDirectory));
  unzipper.EError += Poco::Delegate<SceneIO, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>>(
    this, &SceneIO::OnUnzipError);
  unzipper.EOk += Poco::Delegate<SceneIO, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>>(
    this, &SceneIO::OnUnzipOk);
  unzipper.decompressAllFiles();
  unzipper.EError -= Poco::Delegate<SceneIO, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>>(
    this, &SceneIO::OnUnzipError);
  unzipper.EOk -= Poco::Delegate<SceneIO, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>>(
    this, &SceneIO::OnUnzipOk);

  if (m_UnzipErrors)
  {
    MITK_ERROR << "There were " << m_UnzipErrors << " errors unzipping '" << filename
               << "'. Will attempt to read whatever could be unzipped.";
  }

  // transcode locale-dependent string
  m_WorkingDirectory = Poco::Path::transcode (m_WorkingDirectory);

  // Prefer index.json over index.xml when both exist.
  auto indexJson = m_WorkingDirectory + mitk::IOUtil::GetDirectorySeparator() + "index.json";
  auto indexXml = m_WorkingDirectory + mitk::IOUtil::GetDirectorySeparator() + "index.xml";
  auto indexFile = itksys::SystemTools::FileExists(indexJson.c_str()) ? indexJson : indexXml;
  storage = LoadSceneUnzipped(indexFile, storage, clearStorageFirst);

  // delete temp directory
  try
  {
    Poco::File deleteDir(m_WorkingDirectory);
    deleteDir.remove(true); // recursive
  }
  catch (...)
  {
    MITK_ERROR << "Could not delete temporary directory " << m_WorkingDirectory;
  }

  // return new data storage, even if empty or incomplete (return as much as possible but notify calling method)
  return storage;
}

mitk::DataStorage::Pointer mitk::SceneIO::LoadSceneUnzipped(const std::string &indexfilename,
  DataStorage *pStorage,
  bool clearStorageFirst)
{
  mitk::LocaleSwitch localeSwitch("C");

  // prepare data storage
  DataStorage::Pointer storage = pStorage;
  if (storage.IsNull())
  {
    storage = StandaloneDataStorage::New().GetPointer();
  }

  // test input filename
  if (indexfilename.empty())
  {
    MITK_ERROR << "No filename given. Not possible to load scene.";
    return storage;
  }

  // transcode locale-dependent string
  std::string tempfilename;
  std::string workingDir;
  itksys::SystemTools::SplitProgramPath(indexfilename, workingDir, tempfilename);

  // Route JSON index files to the JSON reader. Clearing is delegated so it
  // can be deferred until the descriptor is validated; a malformed scene
  // file must not wipe the caller's session.
  if (mitk::EndsWithCaseInsensitive(indexfilename, ".json"))
  {
    try
    {
      SceneJsonReader::Pointer jsonReader = SceneJsonReader::New();
      if (!jsonReader->LoadScene(indexfilename, storage, clearStorageFirst))
      {
        MITK_ERROR << "There were errors while loading scene file " << indexfilename
                   << ". Your data may be corrupted";
      }
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "Failed to load JSON scene index '" << indexfilename << "': " << e.what();
    }
    return storage;
  }

  // XML path: clear now (legacy SceneReader has no clearStorageFirst plumbing),
  // then parse the scene descriptor.
  if (clearStorageFirst)
  {
    try
    {
      storage->Remove(storage->GetAll());
    }
    catch (...)
    {
      MITK_ERROR << "DataStorage cannot be cleared properly.";
    }
  }

  tinyxml2::XMLDocument document;
  if (tinyxml2::XML_SUCCESS != document.LoadFile(indexfilename.c_str()))
  {
    MITK_ERROR << "Could not open/read/parse " << workingDir << mitk::IOUtil::GetDirectorySeparator()
      << tempfilename << "\nTinyXML reports: " << document.ErrorStr() << std::endl;
    return storage;
  }

  SceneReader::Pointer reader = SceneReader::New();
  if (!reader->LoadScene(document, workingDir, storage))
  {
    MITK_ERROR << "There were errors while loading scene file " << indexfilename << ". Your data may be corrupted";
  }

  // return new data storage, even if empty or incomplete (return as much as possible but notify calling method)
  return storage;
}

bool mitk::SceneIO::SaveScene(DataStorage::SetOfObjects::ConstPointer sceneNodes,
                              const DataStorage *storage,
                              const std::string &filename)
{
  if (!sceneNodes)
  {
    MITK_ERROR << "No set of nodes given. Not possible to save scene.";
    return false;
  }
  if (!storage)
  {
    MITK_ERROR << "No data storage given. Not possible to save scene."; // \TODO: Technically, it would be possible to
                                                                        // save the nodes without their relation
    return false;
  }

  if (filename.empty())
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
    tinyxml2::XMLDocument document;
    document.InsertEndChild(document.NewDeclaration());

    auto *version = document.NewElement("Version");
    version->SetAttribute("Writer", __FILE__);
    version->SetAttribute("Revision", "$Revision: 17055 $");
    version->SetAttribute("FileVersion", 1);
    document.InsertEndChild(version);

    // DataStorage::SetOfObjects::ConstPointer sceneNodes = storage->GetSubset( predicate );

    if (sceneNodes.IsNull())
    {
      MITK_WARN << "Saving empty scene to " << filename;
    }
    else
    {
      if (sceneNodes->size() == 0)
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

      ProgressBar::GetInstance()->AddStepsToDo(sceneNodes->size());

      // find out about dependencies
      typedef std::map<DataNode *, std::string> UIDMapType;
      typedef std::map<DataNode *, std::list<std::string>> SourcesMapType;

      UIDMapType nodeUIDs;       // for dependencies: ID of each node
      SourcesMapType sourceUIDs; // for dependencies: IDs of a node's parent nodes

      UIDGenerator nodeUIDGen("OBJECT_");

      for (auto iter = sceneNodes->begin(); iter != sceneNodes->end(); ++iter)
      {
        DataNode *node = iter->GetPointer();
        if (!node)
          continue; // unlikely event that we get a nullptr pointer as an object for saving. just ignore

        // generate UIDs for all source objects
        DataStorage::SetOfObjects::ConstPointer sourceObjects = storage->GetSources(node);
        for (auto sourceIter = sourceObjects->begin();
             sourceIter != sourceObjects->end();
             ++sourceIter)
        {
          if (std::find(sceneNodes->begin(), sceneNodes->end(), *sourceIter) == sceneNodes->end())
            continue; // source is not saved, so don't generate a UID for this source

          // create a uid for the parent object
          if (nodeUIDs[*sourceIter].empty())
          {
            nodeUIDs[*sourceIter] = nodeUIDGen.GetUID();
          }

          // store this dependency for writing
          sourceUIDs[node].push_back(nodeUIDs[*sourceIter]);
        }

        if (nodeUIDs[node].empty())
        {
          nodeUIDs[node] = nodeUIDGen.GetUID();
        }
      }

      // write out objects, dependencies and properties
      for (auto iter = sceneNodes->begin(); iter != sceneNodes->end(); ++iter)
      {
        DataNode *node = iter->GetPointer();

        if (node)
        {
          auto *nodeElement = document.NewElement("node");
          std::string filenameHint(node->GetName());
          filenameHint = itksys::SystemTools::MakeCindentifier(
            filenameHint.c_str()); // escape filename <-- only allow [A-Za-z0-9_], replace everything else with _

          // store dependencies
          auto searchUIDIter = nodeUIDs.find(node);
          if (searchUIDIter != nodeUIDs.end())
          {
            // store this node's ID
            nodeElement->SetAttribute("UID", searchUIDIter->second.c_str());
          }

          auto searchSourcesIter = sourceUIDs.find(node);
          if (searchSourcesIter != sourceUIDs.end())
          {
            // store all source IDs
            for (auto sourceUIDIter = searchSourcesIter->second.begin();
                 sourceUIDIter != searchSourcesIter->second.end();
                 ++sourceUIDIter)
            {
              auto *uidElement = document.NewElement("source");
              uidElement->SetAttribute("UID", sourceUIDIter->c_str());
              nodeElement->InsertEndChild(uidElement);
            }
          }

          // store basedata
          if (BaseData *data = node->GetData())
          {
            // std::string filenameHint( node->GetName() );
            bool error(false);
            auto *dataElement = SaveBaseData(document, data, filenameHint, error); // returns a reference to a file
            if (error)
            {
              m_FailedNodes->push_back(node);
            }

            // store basedata properties
            PropertyList *propertyList = data->GetPropertyList();
            if (propertyList && !propertyList->IsEmpty())
            {
              auto *baseDataPropertiesElement =
                SavePropertyList(document, propertyList, nullptr, filenameHint + "-data"); // returns a reference to a file
              if (baseDataPropertiesElement)
                dataElement->InsertEndChild(baseDataPropertiesElement);
            }

            nodeElement->InsertEndChild(dataElement);
          }

          // store all renderwindow specific propertylists
          mitk::DataNode::PropertyListKeyNames propertyListKeys = node->GetPropertyListNames();
          for (const auto &renderWindowName : propertyListKeys)
          {
            PropertyList *propertyList = node->GetPropertyList(renderWindowName);
            if (propertyList && !propertyList->IsEmpty())
            {
              auto *renderWindowPropertiesElement =
                SavePropertyList(document, propertyList, node->GetData(), filenameHint + "-" + renderWindowName); // returns a reference to a file
              if (renderWindowPropertiesElement)
              {
                renderWindowPropertiesElement->SetAttribute("renderwindow", renderWindowName.c_str());
                nodeElement->InsertEndChild(renderWindowPropertiesElement);
              }
            }
          }

          // don't forget the renderwindow independent list
          PropertyList *propertyList = node->GetPropertyList();
          if (propertyList && !propertyList->IsEmpty())
          {
            auto *propertiesElement =
              SavePropertyList(document, propertyList, node->GetData(), filenameHint + "-node"); // returns a reference to a file
            if (propertiesElement)
              nodeElement->InsertEndChild(propertiesElement);
          }
          document.InsertEndChild(nodeElement);
        }
        else
        {
          MITK_WARN << "Ignoring nullptr node during scene serialization.";
        }

        ProgressBar::GetInstance()->Progress();
      } // end for all nodes
    }   // end if sceneNodes

    std::string defaultLocale_WorkingDirectory = Poco::Path::transcode( m_WorkingDirectory );

    auto xmlFilename = defaultLocale_WorkingDirectory + Poco::Path::separator() + "index.xml";
    if (tinyxml2::XML_SUCCESS != document.SaveFile(xmlFilename.c_str()))
    {
      MITK_ERROR << "Could not write scene to " << defaultLocale_WorkingDirectory << Poco::Path::separator() << "index.xml"
                 << "\nTinyXML reports '" << document.ErrorStr() << "'";
      return false;
    }
    else
    {
      try
      {
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
          return false;
        }
        else
        {
          Poco::Zip::Compress zipper(file, true);
          Poco::Path tmpdir(m_WorkingDirectory);
          zipper.addRecursive(tmpdir);
          zipper.close();
        }
        try
        {
          Poco::File deleteDir(m_WorkingDirectory);
          deleteDir.remove(true); // recursive
        }
        catch (...)
        {
          MITK_ERROR << "Could not delete temporary directory " << m_WorkingDirectory;
          return false; // ok?
        }
      }
      catch (std::exception &e)
      {
        MITK_ERROR << "Could not create ZIP file from " << m_WorkingDirectory << "\nReason: " << e.what();
        return false;
      }
      return true;
    }
  }
  catch (std::exception &e)
  {
    MITK_ERROR << "Caught exception during saving temporary files to disk. Error description: '" << e.what() << "'";
    return false;
  }
}

tinyxml2::XMLElement *mitk::SceneIO::SaveBaseData(tinyxml2::XMLDocument &doc, BaseData *data, const std::string &filenamehint, bool &error)
{
  assert(data);
  error = true;

  // find correct serializer
  // the serializer must
  //  - create a file containing all information to recreate the BaseData object --> needs to know where to put this
  //  file (and a filename?)
  //  - TODO what to do about writers that creates one file per timestep?
  auto *element = doc.NewElement("data");
  element->SetAttribute("type", data->GetNameOfClass());

  // construct name of serializer class
  std::string serializername(data->GetNameOfClass());
  serializername += "Serializer";

  std::list<itk::LightObject::Pointer> thingsThatCanSerializeThis =
    itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (thingsThatCanSerializeThis.size() < 1)
  {
    MITK_ERROR << "No serializer found for " << data->GetNameOfClass() << ". Skipping object";
  }

  for (auto iter = thingsThatCanSerializeThis.begin();
       iter != thingsThatCanSerializeThis.end();
       ++iter)
  {
    if (auto *serializer = dynamic_cast<BaseDataSerializer *>(iter->GetPointer()))
    {
      serializer->SetData(data);
      serializer->SetFilenameHint(filenamehint);
      std::string defaultLocale_WorkingDirectory = Poco::Path::transcode( m_WorkingDirectory );
      serializer->SetWorkingDirectory(defaultLocale_WorkingDirectory);
      try
      {
        std::string writtenfilename = serializer->Serialize();
        element->SetAttribute("file", writtenfilename.c_str());

        if (!writtenfilename.empty())
          error = false;
      }
      catch (std::exception &e)
      {
        MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
      }
      break;
    }
  }
  element->SetAttribute("UID", data->GetUID().c_str());

  return element;
}

tinyxml2::XMLElement *mitk::SceneIO::SavePropertyList(tinyxml2::XMLDocument &doc, PropertyList *propertyList, const BaseData *nodeData, const std::string &filenamehint)
{
  assert(propertyList);

  // Drop transient DataNode properties (e.g. the "selected" UI flag) so they are
  // not written to the scene file. Transience is decided per the node's BaseData type.
  PropertyList::Pointer persistable;
  if (nodeData != nullptr)
  {
    CoreServicePointer<IPropertyTransience> transience(CoreServices::GetPropertyTransience());
    persistable = PropertyList::New();

    for (const auto &property : *propertyList->GetMap())
    {
      if (!transience->IsTransient(nodeData, property.first))
        persistable->SetProperty(property.first, property.second);
    }

    if (persistable->IsEmpty())
      return nullptr;

    propertyList = persistable;
  }

  //  - TODO what to do about shared properties (same object in two lists or behind several keys)?
  auto *element = doc.NewElement("properties");

  // construct name of serializer class
  PropertyListSerializer::Pointer serializer = PropertyListSerializer::New();

  serializer->SetPropertyList(propertyList);
  serializer->SetFilenameHint(filenamehint);
  std::string defaultLocale_WorkingDirectory = Poco::Path::transcode( m_WorkingDirectory );
  serializer->SetWorkingDirectory(defaultLocale_WorkingDirectory);
  try
  {
    std::string writtenfilename = serializer->Serialize();
    element->SetAttribute("file", writtenfilename.c_str());
    PropertyList::Pointer failedProperties = serializer->GetFailedProperties();
    if (failedProperties.IsNotNull())
    {
      // move failed properties to global list
      m_FailedProperties->ConcatenatePropertyList(failedProperties, true);
    }
  }
  catch (std::exception &e)
  {
    MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
  }

  return element;
}

const mitk::SceneIO::FailedBaseDataListType *mitk::SceneIO::GetFailedNodes()
{
  return m_FailedNodes.GetPointer();
}

const mitk::PropertyList *mitk::SceneIO::GetFailedProperties()
{
  return m_FailedProperties;
}

void mitk::SceneIO::OnUnzipError(const void * /*pSender*/,
                                 std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> &info)
{
  ++m_UnzipErrors;
  MITK_ERROR << "Error while unzipping: " << info.second;
}

void mitk::SceneIO::OnUnzipOk(const void * /*pSender*/,
                              std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> & /*info*/)
{
  // MITK_INFO << "Unzipped ok: " << info.second.toString();
}
