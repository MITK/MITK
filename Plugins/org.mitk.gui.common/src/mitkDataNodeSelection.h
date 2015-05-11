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

  virtual Object::Pointer GetFirstElement() const override;
  virtual iterator Begin() const override;
  virtual iterator End() const override;

  virtual int Size() const override;

  virtual ContainerType::Pointer ToVector() const override;

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
