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
      \brief tries to create one DataTreeNode from a given XML <node> element
    */
    DataTreeNode::Pointer LoadBaseDataFromDataTag( TiXmlElement* dataElement, 
                                                   const std::string& workingDirectory, 
                                                   bool& error );

    /**
      \brief reads all the properties from the XML document and recreates them in node
    */
    bool DecorateNodeWithProperties(DataTreeNode* node, TiXmlElement* nodeElement, const std::string& workingDirectory);

    typedef std::map<DataTreeNode::Pointer, std::list<std::string> >   NodesAndParentsMapType;
    typedef std::map<std::string, DataTreeNode*> IDToNodeMappingType;
    typedef std::map<DataTreeNode*, std::string> NodeToIDMappingType;

    NodesAndParentsMapType  m_Nodes;
    IDToNodeMappingType     m_NodeForID;
    NodeToIDMappingType     m_IDForNode;

    UIDGenerator m_UIDGen;
};

}

