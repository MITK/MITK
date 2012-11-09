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


#ifndef MITKDATATREENODEOBJECT_H_
#define MITKDATATREENODEOBJECT_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_mitk_gui_common_Export.h>

#include <mitkDataNode.h>

namespace mitk
{

/**
 * \ingroup org_mitk_gui_common
 */
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
