/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSceneReader.h"

namespace tinyxml2
{
  class XMLElement;
}

namespace mitk
{
  class SceneReaderV1 : public SceneReader
  {
  public:
    mitkClassMacro(SceneReaderV1, SceneReader);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    bool LoadScene(tinyxml2::XMLDocument &document,
                             const std::string &workingDirectory,
                             DataStorage *storage) override;

  protected:
    /**
      \brief tries to create one DataNode from a given XML \<node\> element
    */
    DataNode::Pointer LoadBaseDataFromDataTag(const tinyxml2::XMLElement *dataElement,
                                              const std::string &workingDirectory,
                                              bool &error);

    /**
      \brief reads all the properties from the XML document and recreates them in node
    */
    bool DecorateNodeWithProperties(DataNode *node, const tinyxml2::XMLElement *nodeElement, const std::string &workingDirectory);

    /**
      \brief Clear a default property list and handle some exceptions.

      Called after assigning a BaseData object to a fresh DataNode via SetData().
      This call to SetData() would create default properties that have not been
      there when saving the scene. Since they can produce problems, we clear the
      list and use only those properties that we read from the scene file.

      This method also handles some exceptions for backwards compatibility.
      Those exceptions are documented directly in the code of the method.
    */
    void ClearNodePropertyListWithExceptions(DataNode &node, PropertyList &propertyList);

    /**
      \brief reads all properties assigned to a base data element and assigns the list to the base data object

      The baseDataNodeElem is supposed to be the \c \<properties file="..."\> element.
    */
    bool DecorateBaseDataWithProperties(BaseData::Pointer data,
                                        const tinyxml2::XMLElement *baseDataNodeElem,
                                        const std::string &workingDir);

    typedef std::pair<DataNode::Pointer, std::list<std::string>> NodesAndParentsPair;
    typedef std::list<NodesAndParentsPair> OrderedNodesList;
    typedef std::map<std::string, DataNode *> IDToNodeMappingType;
    typedef std::map<DataNode *, std::string> NodeToIDMappingType;

    OrderedNodesList m_OrderedNodePairs;
    IDToNodeMappingType m_NodeForID;
    NodeToIDMappingType m_IDForNode;

    UIDGenerator m_UIDGen;
  };
}
