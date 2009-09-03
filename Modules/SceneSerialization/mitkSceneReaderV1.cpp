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

#include "mitkSceneReaderV1.h"
#include "mitkSerializerMacros.h"

MITK_REGISTER_SERIALIZER(SceneReaderV1)
    
bool mitk::SceneReaderV1::LoadScene( TiXmlDocument& document, const std::string& workingDirectory, DataStorage* storage )
{
  assert(storage);

  LOG_INFO << "Reading objects for MITK scene from '" << workingDirectory << "'";

  // iterate all nodes

  // first level nodes should be <node> elements. for each
  //   1. create a new, empty node
  //   2. check child nodes
  //   3. if there is a <source>, remember parent objects
  //   4. if there is a <data type="..." file="..."> element,
  //        - construct a name for the appropriate deserializer
  //        - try to instantiate this deserializer via itk object factory
  //        - if deserializer could be created, use it to read the file into a BaseData object
  //        - if successful, call the new node's SetData(..)
  //   5. if there are <properties> nodes, 
  //        - instantiate the appropriate PropertyListDeSerializer
  //        - use them to construct PropertyList objects
  //        - add these properties to the node (if necessary, use renderwindow name)

  // repeat
  //   for all created nodes
  //     find all nodes that
  //      - do not have any parents or
  //      - have all their parents already in DataStorage
  //     add these to datastorage
  // repeat above block until there are no nodes left for adding
  
  // prepare to detect errors (such as cycles) from wrongly written or edited xml files
  
  LOG_INFO << "Not yet implemented";

  return false;
}

