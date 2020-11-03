/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

  berryObjectMacro(mitk::DataNodeObject);

  DataNodeObject();
  DataNodeObject(DataNode::Pointer node);

  DataNode::Pointer GetDataNode() const;

  bool operator==(const berry::Object* obj) const override;

private:

  DataNode::Pointer m_Node;


};

}

#endif /* MITKDATATREENODEOBJECT_H_ */
