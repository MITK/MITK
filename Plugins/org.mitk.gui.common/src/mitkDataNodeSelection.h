/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKDATATREENODESELECTION_H_
#define MITKDATATREENODESELECTION_H_

#include <berryIStructuredSelection.h>
#include <mitkDataNode.h>
#include <org_mitk_gui_common_Export.h>


namespace mitk {

/**
 * \ingroup org_mitk_gui_common
 */
class MITK_GUI_COMMON_PLUGIN DataNodeSelection : public virtual berry::IStructuredSelection
{

public:

  berryObjectMacro(DataNodeSelection);

  DataNodeSelection();
  DataNodeSelection(DataNode::Pointer node);
  DataNodeSelection(const std::vector<DataNode::Pointer>& nodes);

  Object::Pointer GetFirstElement() const override;
  iterator Begin() const override;
  iterator End() const override;

  int Size() const override;

  ContainerType::Pointer ToVector() const override;

  std::list<mitk::DataNode::Pointer> GetSelectedDataNodes() const;

  /**
   * @see berry::ISelection::IsEmpty()
   */
  bool IsEmpty() const override;

  bool operator==(const berry::Object* obj) const override;

protected:

  ContainerType::Pointer m_Selection;
};

}


#endif /* MITKDATATREENODESELECTION_H_ */
