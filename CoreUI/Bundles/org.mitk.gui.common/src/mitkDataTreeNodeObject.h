/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef MITKDATATREENODEOBJECT_H_
#define MITKDATATREENODEOBJECT_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "mitkGUICommonPluginDll.h"

#include <mitkDataTreeNode.h>

namespace mitk
{

class MITK_GUI_COMMON_PLUGIN DataTreeNodeObject : public cherry::Object
{

public:

  cherryObjectMacro(mitk::DataTreeNodeObject)

  DataTreeNodeObject();
  DataTreeNodeObject(DataTreeNode::Pointer node);

  DataTreeNode::Pointer GetDataTreeNode() const;

  bool operator==(const cherry::Object* obj) const;

private:

  DataTreeNode::Pointer m_Node;


};

}

#endif /* MITKDATATREENODEOBJECT_H_ */
