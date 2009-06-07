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


#ifndef MITKDATATREENODESELECTION_H_
#define MITKDATATREENODESELECTION_H_

#include <cherryIStructuredSelection.h>
#include <mitkDataTreeNode.h>
#include "mitkGUICommonPluginDll.h"


namespace mitk {

class MITK_GUI_COMMON_PLUGIN DataTreeNodeSelection : public virtual cherry::IStructuredSelection
{

public:

  cherryObjectMacro(DataTreeNodeSelection);

  DataTreeNodeSelection();
  DataTreeNodeSelection(DataTreeNode::Pointer node);
  DataTreeNodeSelection(const std::vector<DataTreeNode::Pointer>& nodes);

  virtual Object::Pointer GetFirstElement() const;
  virtual iterator Begin() const;
  virtual iterator End() const;

  virtual int Size() const;

  virtual ContainerType::Pointer ToVector() const;

  /**
   * @see cherry::ISelection::IsEmpty()
   */
  bool IsEmpty() const;

  bool operator==(const cherry::Object* obj) const;

protected:

  ContainerType::Pointer m_Selection;
};

}


#endif /* MITKDATATREENODESELECTION_H_ */
