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

#include "mitkSceneReaderV1.h"
#include "mitkSerializerMacros.h"
#include "mitkBaseRenderer.h"
#include "mitkPropertyListDeserializer.h"
#include "mitkProgressBar.h"
#include "mitkIOUtil.h"
#include "Poco/Path.h"
#include <mitkRenderingModeProperty.h>

MITK_REGISTER_SERIALIZER(SceneReaderV1)

bool mitk::SceneReaderV1::LoadScene(TiXmlDocument& document, const std::string& workingDirectory, DataStorage* storage, LoadedNodeFileNamesMap* nodeDataFileNameMap)
{
  assert(storage);
  bool error(false);

  // TODO prepare to detect errors (such as cycles) from wrongly written or edited xml files

  //Get number of elements to initialze progress bar
  //   1. if there is a <data type="..." file="..."> element,
    //        - construct a name for the appropriate serializer
    //        - try to instantiate this serializer via itk object factory
    //        - if serializer could be created, use it to read the file into a BaseData object
    //        - if successful, call the new node's SetData(..)

    // create a node for the tag "data" and test if node was created
  typedef std::vector<mitk::DataNode::Pointer> DataNodeVector;
  DataNodeVector DataNodes;
  unsigned int listSize = 0;
  for( TiXmlElement* element = document.FirstChildElement("node"); element != NULL; element = element->NextSiblingElement("node") )
  {
    ++listSize;
  }

  ProgressBar::GetInstance()->AddStepsToDo(listSize * 2);

  for (TiXmlElement* element = document.FirstChildElement("node"); element != NULL; element = element->NextSiblingElement("node"))
  {
      mitk::DataNode::Pointer newNode = LoadBaseDataFromDataTag(element->FirstChildElement("data"), workingDirectory, error, nodeDataFileNameMap);
      DataNodes.push_back(newNode);
      ProgressBar::GetInstance()->Progress();
  }

  OrderedLayers orderedLayers;
  this->GetLayerOrder(document, workingDirectory, DataNodes, orderedLayers, nodeDataFileNameMap);


  // iterate all nodes
  // first level nodes should be <node> elements
  DataNodeVector::iterator nit = DataNodes.begin();
  for( TiXmlElement* element = document.FirstChildElement("node"); element != NULL || nit != DataNodes.end(); element = element->NextSiblingElement("node"), ++nit )
  {
    mitk::DataNode::Pointer node = *nit;
    // in case dataXmlElement is valid test whether it containts the "properties" child tag
    // and process further if and only if yes
    TiXmlElement *dataXmlElement = element->FirstChildElement("data");
    if( dataXmlElement && dataXmlElement->FirstChildElement("properties") )
    {
      TiXmlElement *baseDataElement = dataXmlElement->FirstChildElement("properties");
      if ( node->GetData() )
      {
        DecorateBaseDataWithProperties( node, baseDataElement, workingDirectory, nodeDataFileNameMap);
      }
      else
      {
        MITK_WARN << "BaseData properties stored in scene file, but BaseData can't be read" << std::endl;
      }
    }

    //   2. check child nodes
    const char* uida = element->Attribute("UID");
    std::string uid("");

    if (uida)
    {
      uid = uida;
      m_NodeForID[uid] = node.GetPointer();
      m_IDForNode[ node.GetPointer() ] = uid;
    }
    else
    {
      MITK_ERROR << "No UID found for current node. Node will have no parents.";
      error = true;
    }

    //   3. if there are <properties> nodes,
    //        - instantiate the appropriate PropertyListDeSerializer
    //        - use them to construct PropertyList objects
    //        - add these properties to the node (if necessary, use renderwindow name)
    bool success = DecorateNodeWithProperties(node, element, workingDirectory, nodeDataFileNameMap);
    if (!success)
    {
      MITK_ERROR << "Could not load properties for node.";
      error = true;
    }

    // remember node for later adding to DataStorage

    //node->GetIntProperty("layer", layer);

    int layer;
    OrderedLayers::iterator it = orderedLayers.find(uid);
    layer = (*it).second;

    m_OrderedNodePairs.insert( std::make_pair( layer, std::make_pair( node, std::list<std::string>() ) ) );

    //   4. if there are <source> elements, remember parent objects
    for( TiXmlElement* source = element->FirstChildElement("source"); source != NULL; source = source->NextSiblingElement("source") )
    {
      const char* sourceUID = source->Attribute("UID");
      if (sourceUID)
      {
        m_OrderedNodePairs[layer].second.push_back( std::string(sourceUID) );
      }
    }

    ProgressBar::GetInstance()->Progress();

  } // end for all <node>

  // remove all unknown parent UIDs
  for (LayerPropertyMapType::iterator nodesIter = m_OrderedNodePairs.begin();
       nodesIter != m_OrderedNodePairs.end();
       ++nodesIter)
  {
    for (std::list<std::string>::iterator parentsIter = nodesIter->second.second.begin();
         parentsIter != nodesIter->second.second.end();)
    {
      if (m_NodeForID.find( *parentsIter ) == m_NodeForID.end())
      {
        parentsIter = nodesIter->second.second.erase( parentsIter );
        MITK_WARN << "Found a DataNode with unknown parents. Will add it to DataStorage without any parent objects.";
        error = true;
      }
      else
      {
        ++parentsIter;
      }
    }
  }

  // repeat
  //   for all created nodes
  unsigned int lastMapSize(0);
  while ( lastMapSize != m_OrderedNodePairs.size()) // this is to prevent infinite loops; each iteration must at least add one node to DataStorage
  {
    lastMapSize = m_OrderedNodePairs.size();

    for (LayerPropertyMapType::iterator nodesIter = m_OrderedNodePairs.begin();
         nodesIter != m_OrderedNodePairs.end();
         ++nodesIter)
    {
      bool addNow(true);
      // if any parent node is not yet in DataStorage, skip node for now and check later
      for (std::list<std::string>::iterator parentsIter = nodesIter->second.second.begin();
           parentsIter != nodesIter->second.second.end();
           ++parentsIter)
      {
        if ( !storage->Exists( m_NodeForID[ *parentsIter ] ) )
        {
          addNow = false;
          break;
        }
      }

      if (addNow)
      {
        DataStorage::SetOfObjects::Pointer parents = DataStorage::SetOfObjects::New();
        for (std::list<std::string>::iterator parentsIter = nodesIter->second.second.begin();
             parentsIter != nodesIter->second.second.end();
             ++parentsIter)
        {
          parents->push_back( m_NodeForID[ *parentsIter ] );
        }

        // if all parents are found in datastorage (or are unknown), add node to DataStorage
        storage->Add( nodesIter->second.first, parents );

        // remove this node from m_OrderedNodePairs
        m_OrderedNodePairs.erase( nodesIter );

        // break this for loop because iterators are probably invalid
        break;
      }
    }
  }

  // All nodes that are still in m_OrderedNodePairs at this point are not part of a proper directed graph structure. We'll add such nodes without any parent information.
  for (LayerPropertyMapType::iterator nodesIter = m_OrderedNodePairs.begin();
       nodesIter != m_OrderedNodePairs.end();
       ++nodesIter)
  {
    storage->Add( nodesIter->second.first );
    MITK_WARN << "Encountered node that is not part of a directed graph structure. Will be added to DataStorage without parents.";
    error = true;
  }

  return !error;
}

void mitk::SceneReaderV1::GetLayerOrder(TiXmlDocument& document, const std::string& workingDirectory, std::vector<mitk::DataNode::Pointer> DataNodes, OrderedLayers& order, LoadedNodeFileNamesMap* nodeDataFileNameMap)
{
  typedef std::vector<mitk::DataNode::Pointer> DataNodeVector;
  DataNodeVector::iterator nit = DataNodes.begin();
  for( TiXmlElement* element = document.FirstChildElement("node"); element != NULL || nit != DataNodes.end(); element = element->NextSiblingElement("node"), ++nit )
  {
    DataNode::Pointer node = *nit;
    DecorateNodeWithProperties(node, element, workingDirectory, nodeDataFileNameMap);

    int layer;
    node->GetIntProperty("layer", layer);
    std::string uid = element->Attribute("UID");
    this->m_UnorderedLayers.insert( std::make_pair( layer, uid ) );
  }

  int lastLayer = itk::NumericTraits<int>::min();
  UnorderedLayers::iterator it;
  for (it = m_UnorderedLayers.begin(); it != m_UnorderedLayers.end(); it++)
  {
    int currentLayer = (*it).first;
    if (currentLayer == lastLayer)
    {
      ++currentLayer;
    }
    order.insert( std::make_pair( (*it).second, currentLayer ) );
    lastLayer = currentLayer;
  }
}

mitk::DataNode::Pointer mitk::SceneReaderV1::LoadBaseDataFromDataTag(TiXmlElement* dataElement, const std::string& workingDirectory, bool& error, LoadedNodeFileNamesMap* nodeDataFileNameMap)
{
  DataNode::Pointer node;

  if (dataElement)
  {
    const char* filename = dataElement->Attribute("file");
    if ( filename )
    {
      try
      {
        std::vector<BaseData::Pointer> baseData = IOUtil::Load( workingDirectory + Poco::Path::separator() + filename );
        if (baseData.size() > 1)
        {
          MITK_WARN << "Discarding multiple base data results from " << filename << " except the first one.";
        }
        node = DataNode::New();
        node->SetData(baseData.front());

        if (nodeDataFileNameMap) {
            (*nodeDataFileNameMap)[node].baseDataFiles.clear();
            (*nodeDataFileNameMap)[node].baseDataFiles.push_back(filename);

            for (TiXmlElement* element = dataElement->FirstChildElement("additionalFile"); element != NULL; element = element->NextSiblingElement("additionalFile"))
            {
                (*nodeDataFileNameMap)[node].baseDataFiles.push_back(element->Attribute("file"));
            }

        }
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Error during attempt to read '" << filename << "'. Exception says: " << e.what();
        error = true;
      }

      if (node.IsNull())
      {
        MITK_ERROR << "Error during attempt to read '" << filename << "'. Factory returned NULL object.";
        error = true;
      }
    }
  }

  // in case there was no <data> element we create a new empty node (for appending a propertylist later)
  if (node.IsNull())
  {
    node = DataNode::New();
  }

  return node;
}

bool mitk::SceneReaderV1::DecorateNodeWithProperties(DataNode* node, TiXmlElement* nodeElement, const std::string& workingDirectory, LoadedNodeFileNamesMap* nodeDataFileNameMap)
{
  assert(node);
  assert(nodeElement);
  bool error(false);

  for( TiXmlElement* properties = nodeElement->FirstChildElement("properties"); properties != NULL; properties = properties->NextSiblingElement("properties") )
  {
    const char* propertiesfilea( properties->Attribute("file") );
    std::string propertiesfile( propertiesfilea ? propertiesfilea : "" );

    const char* renderwindowa( properties->Attribute("renderwindow") );
    std::string renderwindow( renderwindowa ? renderwindowa : "" );

    PropertyList::Pointer propertyList = node->GetPropertyList(renderwindow); // DataNode implementation always returns a propertylist
    // clear all properties from node that might be set by DataNodeFactory during loading
    propertyList->Clear();

    // use deserializer to construct new properties
    PropertyListDeserializer::Pointer deserializer = PropertyListDeserializer::New();

    deserializer->SetFilename(workingDirectory + Poco::Path::separator() + propertiesfile);
    bool success = deserializer->Deserialize();
    error |= !success;
    PropertyList::Pointer readProperties = deserializer->GetOutput();

    if (readProperties.IsNotNull())
    {
      propertyList->ConcatenatePropertyList( readProperties, true ); // true = replace
      if (nodeDataFileNameMap) {
          (*nodeDataFileNameMap)[node].nodePropertiesFiles[renderwindow] = propertiesfile;
      }
    }
    else
    {
      MITK_ERROR << "Property list reader did not return a property list. This is an implementation error. Please tell your developer.";
      error = true;
    }
  }

  return !error;
}

bool mitk::SceneReaderV1::DecorateBaseDataWithProperties(DataNode* node, TiXmlElement *baseDataNodeElem, const std::string &workingDir, LoadedNodeFileNamesMap* nodeDataFileNameMap)
{
  // check given variables, initialize error variable
  assert(baseDataNodeElem);
  bool error(false);

  // get the file name stored in the <properties ...> tag
  const char* baseDataPropertyFile( baseDataNodeElem->Attribute("file") );
  // check if the filename was found
  if(baseDataPropertyFile)
  {
    //PropertyList::Pointer dataPropList = data->GetPropertyList();

    PropertyListDeserializer::Pointer propertyDeserializer = PropertyListDeserializer::New();

    // initialize the property reader
    propertyDeserializer->SetFilename(workingDir + Poco::Path::separator() + baseDataPropertyFile);
    bool ioSuccess = propertyDeserializer->Deserialize();
    error = !ioSuccess;

    // get the output
    PropertyList::Pointer inProperties = propertyDeserializer->GetOutput();

    // store the read-in properties to the given node or throw error otherwise
    if( inProperties.IsNotNull() )
    {
      node->GetData()->SetPropertyList( inProperties );
      if (nodeDataFileNameMap) {
          (*nodeDataFileNameMap)[node].baseDataPropertiesFile = baseDataPropertyFile;
      }
    }
    else
    {
      MITK_ERROR << "The property deserializer did not return a (valid) property list.";
      error = true;
    }
  }
  else
  {
    MITK_ERROR << "Function DecorateBaseDataWithProperties(...) called with false TiXmlElement. \n \t ->Given element does not contain a 'file' attribute. \n";
    error = true;
  }

  return !error;
}

