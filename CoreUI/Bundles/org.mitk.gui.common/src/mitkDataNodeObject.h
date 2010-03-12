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

#include <berryObject.h>
#include <berryMacros.h>

#include "mitkGUICommonPluginDll.h"

#include <mitkDataNode.h>

namespace mitk
{

class MITK_GUI_COMMON_PLUGIN DataNodeObject : public berry::Object
{

public:

  berryObjectMacro(mitk::DataNodeObject)

  DataNodeObject();
  DataNodeObject(DataNode::Pointer node);

  DataNode::Pointer GetDataNode() const;

  bool operator==(const berry::Object* obj) const;

private:

  DataNode::Pointer m_Node;


};

}

#endif /* MITKDATATREENODEOBJECT_H_ */
