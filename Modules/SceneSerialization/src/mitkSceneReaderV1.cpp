/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSceneReaderV1.h"
#include "Poco/Path.h"
#include "mitkBaseRenderer.h"
#include "mitkIOUtil.h"
#include "mitkProgressBar.h"
#include "mitkPropertyListDeserializer.h"
#include "mitkSerializerMacros.h"
#include <mitkUIDManipulator.h>
#include <mitkRenderingModeProperty.h>
#include <tinyxml2.h>

#include <filesystem>
#include <map>

MITK_REGISTER_SERIALIZER(SceneReaderV1)

namespace
{
  typedef std::pair<mitk::DataNode::Pointer, std::list<std::string>> NodesAndParentsPair;

  bool NodeSortByLayerIsLessThan(const NodesAndParentsPair &left, const NodesAndParentsPair &right)
  {
    if (left.first.IsNotNull() && right.first.IsNotNull())
    {
      int leftLayer;
      int rightLayer;
      if (left.first->GetIntProperty("layer", leftLayer) && right.first->GetIntProperty("layer", rightLayer))
      {
        return leftLayer < rightLayer;
      }
      else
      {
        // fall back to name sort
        return left.first->GetName() < right.first->GetName();
      }
    }

    // in all other cases, fall back to stupid pointer comparison
    // this is not reasonable but at least answers the sorting
    // question clearly
    return left.first.GetPointer() < right.first.GetPointer();
  }

  // This is a workaround until we are able to save time-related information in an
  // actual file format of surfaces.
  void ApplyProportionalTimeGeometryProperties(mitk::BaseData* data)
  {
    auto* geometry = dynamic_cast<mitk::ProportionalTimeGeometry*>(data->GetTimeGeometry());

    if (nullptr == geometry)
      return;

    auto properties = data->GetPropertyList();
    float value = 0.0f;

    if (properties->GetFloatProperty("ProportionalTimeGeometry.FirstTimePoint", value))
    {
      if (value == -std::numeric_limits<float>::infinity())
        value = std::numeric_limits<float>::lowest();

      geometry->SetFirstTimePoint(value);
    }

    if (properties->GetFloatProperty("ProportionalTimeGeometry.StepDuration", value))
      geometry->SetStepDuration(value);
  }

  mitk::PropertyList::Pointer DeserializeProperties(const tinyxml2::XMLElement *propertiesElement, const std::filesystem::path& basePath)
  {
    if (propertiesElement == nullptr)
      return nullptr;

    std::filesystem::path path(propertiesElement->Attribute("file"));

    if (path.empty())
      return nullptr;

    if (!basePath.empty())
      path = basePath / path;

    auto deserializer = mitk::PropertyListDeserializer::New();
    deserializer->SetFilename(path.string());
    deserializer->Deserialize();

    return deserializer->GetOutput();
  }
}

bool mitk::SceneReaderV1::LoadScene(tinyxml2::XMLDocument &document, const std::string &workingDirectory, DataStorage *storage)
{
  assert(storage);
  bool error(false);

  // TODO prepare to detect errors (such as cycles) from wrongly written or edited xml files

  // Get number of elements to initialze progress bar
  //   1. if there is a <data type="..." file="..."> element,
  //        - construct a name for the appropriate serializer
  //        - try to instantiate this serializer via itk object factory
  //        - if serializer could be created, use it to read the file into a BaseData object
  //        - if successful, call the new node's SetData(..)

  // create a node for the tag "data" and test if node was created
  typedef std::vector<mitk::DataNode::Pointer> DataNodeVector;
  DataNodeVector DataNodes;
  unsigned int listSize = 0;
  for (auto *element = document.FirstChildElement("node"); element != nullptr;
       element = element->NextSiblingElement("node"))
  {
    ++listSize;
  }

  ProgressBar::GetInstance()->AddStepsToDo(listSize * 2);

  // Deserialize base data properties before reading the actual data to be
  // able to provide them as read-only meta data to the data reader.
  std::map<std::string, PropertyList::Pointer> baseDataPropertyLists;

  for (auto *nodeElement = document.FirstChildElement("node"); nodeElement != nullptr;
       nodeElement = nodeElement->NextSiblingElement("node"))
  {
    const auto *uid = nodeElement->Attribute("UID");

    if (uid == nullptr)
      continue;

    auto *dataElement = nodeElement->FirstChildElement("data");

    if (dataElement != nullptr)
    {
      auto properties = DeserializeProperties(dataElement->FirstChildElement("properties"), workingDirectory);

      if (properties.IsNotNull())
        baseDataPropertyLists[uid] = properties;
    }
  }

  for (auto *element = document.FirstChildElement("node"); element != nullptr;
       element = element->NextSiblingElement("node"))
  {
    mitk::PropertyList* properties = nullptr;
    const auto *uid = element->Attribute("UID");

    if (uid != nullptr)
    {
      auto iter = baseDataPropertyLists.find(uid);

      if (iter != baseDataPropertyLists.end())
        properties = iter->second;
    }

    const auto *dataElement = element->FirstChildElement("data");
    auto dataNode = this->LoadBaseDataFromDataTag(dataElement, properties, workingDirectory, error);

    if (dataNode.IsNull())
      continue;

    auto* baseData = dataNode->GetData();

    if (baseData != nullptr && properties != nullptr)
    {
      baseData->SetPropertyList(properties);
      ApplyProportionalTimeGeometryProperties(baseData);
    }

    DataNodes.push_back(dataNode);

    ProgressBar::GetInstance()->Progress();
  }

  // iterate all nodes
  // first level nodes should be <node> elements
  auto nit = DataNodes.begin();
  for (auto *element = document.FirstChildElement("node"); element != nullptr || nit != DataNodes.end();
       element = element->NextSiblingElement("node"), ++nit)
  {
    mitk::DataNode::Pointer node = *nit;

    //   1. check child nodes
    const char *uida = element->Attribute("UID");
    std::string uid("");

    if (uida)
    {
      uid = uida;
      m_NodeForID[uid] = node.GetPointer();
      m_IDForNode[node.GetPointer()] = uid;
    }
    else
    {
      MITK_ERROR << "No UID found for current node. Node will have no parents.";
      error = true;
    }

    //   2. if there are <properties> nodes,
    //        - instantiate the appropriate PropertyListDeSerializer
    //        - use them to construct PropertyList objects
    //        - add these properties to the node (if necessary, use renderwindow name)
    bool success = DecorateNodeWithProperties(node, element, workingDirectory);
    if (!success)
    {
      MITK_ERROR << "Could not load properties for node.";
      error = true;
    }

    // remember node for later adding to DataStorage
    m_OrderedNodePairs.push_back(std::make_pair(node, std::list<std::string>()));

    //   3. if there are <source> elements, remember parent objects
    for (auto *source = element->FirstChildElement("source"); source != nullptr;
         source = source->NextSiblingElement("source"))
    {
      const char *sourceUID = source->Attribute("UID");
      if (sourceUID)
      {
        m_OrderedNodePairs.back().second.push_back(std::string(sourceUID));
      }
    }

    ProgressBar::GetInstance()->Progress();
  } // end for all <node>

  // sort our nodes by their "layer" property
  // (to be inserted in that order)
  m_OrderedNodePairs.sort(&NodeSortByLayerIsLessThan);

  // remove all unknown parent UIDs
  for (auto nodesIter = m_OrderedNodePairs.begin(); nodesIter != m_OrderedNodePairs.end();
       ++nodesIter)
  {
    for (auto parentsIter = nodesIter->second.begin();
         parentsIter != nodesIter->second.end();)
    {
      if (m_NodeForID.find(*parentsIter) == m_NodeForID.end())
      {
        parentsIter = nodesIter->second.erase(parentsIter);
        MITK_WARN << "Found a DataNode with unknown parents. Will add it to DataStorage without any parent objects.";
        error = true;
      }
      else
      {
        ++parentsIter;
      }
    }
  }

  // repeat the following loop ...
  //   ... for all created nodes
  unsigned int lastMapSize(0);
  while (lastMapSize !=
         m_OrderedNodePairs
           .size()) // this is to prevent infinite loops; each iteration must at least add one node to DataStorage
  {
    lastMapSize = m_OrderedNodePairs.size();

    // iterate (layer) ordered nodes backwards
    // we insert the highest layers first
    for (auto nodesIter = m_OrderedNodePairs.begin(); nodesIter != m_OrderedNodePairs.end();
         ++nodesIter)
    {
      bool addThisNode(true);

      // if any parent node is not yet in DataStorage, skip node for now and check later
      for (auto parentsIter = nodesIter->second.begin();
           parentsIter != nodesIter->second.end();
           ++parentsIter)
      {
        if (!storage->Exists(m_NodeForID[*parentsIter]))
        {
          addThisNode = false;
          break;
        }
      }

      if (addThisNode)
      {
        DataStorage::SetOfObjects::Pointer parents = DataStorage::SetOfObjects::New();
        for (auto parentsIter = nodesIter->second.begin();
             parentsIter != nodesIter->second.end();
             ++parentsIter)
        {
          parents->push_back(m_NodeForID[*parentsIter]);
        }

        // if all parents are found in datastorage (or are unknown), add node to DataStorage
        storage->Add(nodesIter->first, parents);

        // remove this node from m_OrderedNodePairs
        m_OrderedNodePairs.erase(nodesIter);

        // break this for loop because iterators are probably invalid
        break;
      }
    }
  }

  // All nodes that are still in m_OrderedNodePairs at this point are not part of a proper directed graph structure.
  // We'll add such nodes without any parent information.
  for (auto nodesIter = m_OrderedNodePairs.begin(); nodesIter != m_OrderedNodePairs.end();
       ++nodesIter)
  {
    storage->Add(nodesIter->first);
    MITK_WARN << "Encountered node that is not part of a directed graph structure. Will be added to DataStorage "
                 "without parents.";
    error = true;
  }

  return !error;
}

mitk::DataNode::Pointer mitk::SceneReaderV1::LoadBaseDataFromDataTag(const tinyxml2::XMLElement *dataElement,
                                                                     const PropertyList *properties,
                                                                     const std::string &workingDirectory,
                                                                     bool &error)
{
  DataNode::Pointer node;

  if (dataElement)
  {
    const char *filename = dataElement->Attribute("file");
    if (filename && strlen(filename) != 0)
    {
      try
      {
        auto baseData = IOUtil::Load(workingDirectory + Poco::Path::separator() + filename, properties);

        node = DataNode::New();
        node->SetData(baseData);
      }
      catch (std::exception &e)
      {
        MITK_ERROR << "Error during attempt to read '" << filename << "'. Exception says: " << e.what();
        error = true;
      }

      if (node.IsNull())
      {
        MITK_ERROR << "Error during attempt to read '" << filename << "'. Factory returned nullptr object.";
        error = true;
      }
    }
    else
    {
      MITK_ERROR << "File attribute of data tag is empty!";
      error = true;
    }

    const char* dataUID = dataElement->Attribute("UID");
    if (!error && dataUID != nullptr)
    {
      UIDManipulator manip(node->GetData());
      manip.SetUID(dataUID);
    }
  }

  // in case there was no <data> element we create a new empty node (for appending a propertylist later)
  if (node.IsNull())
  {
    node = DataNode::New();
  }

  return node;
}

void mitk::SceneReaderV1::ClearNodePropertyListWithExceptions(DataNode &node, PropertyList &propertyList)
{
  // Basically call propertyList.Clear(), but implement exceptions (see bug 19354)
  BaseData *data = node.GetData();

  PropertyList::Pointer propertiesToKeep = PropertyList::New();

  if (dynamic_cast<Image *>(data))
  {
    /*
      Older scene files (before changes of bug 17547) could contain
      a RenderingMode property with value "LevelWindow_Color".
      Since bug 17547 this value has been removed and replaced by
      the default value LookupTable_LevelWindow_Color.

      This new default value does only result in "black-to-white"
      CT images (or others) if there is a corresponding lookup
      table. Such a lookup table is provided as a default value
      by the Image mapper. Since that value was never present in
      older scene files, we do well in not removing the new
      default value here. Otherwise the mapper would fall back
      to another default which is all the colors of the rainbow :-(
    */
    BaseProperty::Pointer lutProperty = propertyList.GetProperty("LookupTable");
    propertiesToKeep->SetProperty("LookupTable", lutProperty);

    /*
      Older scene files (before changes of T14807) may contain
      multi-component images without the "Image.Displayed Component"
      property.

      As the treatment as multi-component image and the corresponding
      visualization options hinges on that property we should not delete
      it, if it was added by the mapper.

      This is a fix for the issue reported in T19919.
    */
    BaseProperty::Pointer compProperty = propertyList.GetProperty("Image.Displayed Component");
    if (compProperty.IsNotNull())
    {
      propertiesToKeep->SetProperty("Image.Displayed Component", compProperty);
    }
  }

  propertyList.Clear();

  propertyList.ConcatenatePropertyList(propertiesToKeep);
}

bool mitk::SceneReaderV1::DecorateNodeWithProperties(DataNode *node,
                                                     const tinyxml2::XMLElement *nodeElement,
                                                     const std::string &workingDirectory)
{
  assert(node);
  assert(nodeElement);
  bool error(false);

  for (auto *properties = nodeElement->FirstChildElement("properties"); properties != nullptr;
       properties = properties->NextSiblingElement("properties"))
  {
    const char *propertiesfilea(properties->Attribute("file"));
    std::string propertiesfile(propertiesfilea ? propertiesfilea : "");

    const char *renderwindowa(properties->Attribute("renderwindow"));
    std::string renderwindow(renderwindowa ? renderwindowa : "");

    PropertyList::Pointer propertyList =
      node->GetPropertyList(renderwindow); // DataNode implementation always returns a propertylist
    ClearNodePropertyListWithExceptions(*node, *propertyList);

    // use deserializer to construct new properties
    PropertyListDeserializer::Pointer deserializer = PropertyListDeserializer::New();

    deserializer->SetFilename(workingDirectory + Poco::Path::separator() + propertiesfile);
    bool success = deserializer->Deserialize();
    error |= !success;
    PropertyList::Pointer readProperties = deserializer->GetOutput();

    if (readProperties.IsNotNull())
    {
      propertyList->ConcatenatePropertyList(readProperties, true); // true = replace
    }
    else
    {
      MITK_ERROR << "Property list reader did not return a property list. This is an implementation error. Please tell "
                    "your developer.";
      error = true;
    }
  }

  return !error;
}
