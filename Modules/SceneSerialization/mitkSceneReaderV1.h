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

#include "mitkSceneReader.h"

namespace mitk
{

class SceneReaderV1 : public SceneReader
{
  public:

    mitkClassMacro( SceneReaderV1, SceneReader);
    itkNewMacro( Self );

    virtual bool LoadScene( TiXmlDocument& document, const std::string& workingDirectory, DataStorage* storage );

  protected:

    /**
      \brief tries to create one DataNode from a given XML <node> element
    */
    DataNode::Pointer LoadBaseDataFromDataTag( TiXmlElement* dataElement,
                                                   const std::string& workingDirectory,
                                                   bool& error );

    /**
      \brief reads all the properties from the XML document and recreates them in node
    */
    bool DecorateNodeWithProperties(DataNode* node, TiXmlElement* nodeElement, const std::string& workingDirectory);

    /**
      \brief reads all properties assigned to a base data element and assigns the list to the base data object

      The baseDataNodeElem is supposed to be the <properties file="..."> element.
    */
    bool DecorateBaseDataWithProperties(BaseData::Pointer data, TiXmlElement* baseDataNodeElem, const std::string& workingDir);

    typedef std::multimap<int, std::string> UnorderedLayers;
    typedef std::map<std::string, int> OrderedLayers;
    typedef std::pair<DataNode::Pointer, std::list<std::string> >   NodesAndParentsPair;
    typedef std::map<int, NodesAndParentsPair >   LayerPropertyMapType;
    typedef std::map<std::string, DataNode*> IDToNodeMappingType;
    typedef std::map<DataNode*, std::string> NodeToIDMappingType;

    void GetLayerOrder(TiXmlDocument& document, const std::string& workingDirectory, std::vector<mitk::DataNode::Pointer> DataNodes, OrderedLayers& order);

    UnorderedLayers         m_UnorderedLayers;
    OrderedLayers           m_OrderedLayers;
    LayerPropertyMapType    m_OrderedNodePairs;
    IDToNodeMappingType     m_NodeForID;
    NodeToIDMappingType     m_IDForNode;

    UIDGenerator m_UIDGen;
};

}

