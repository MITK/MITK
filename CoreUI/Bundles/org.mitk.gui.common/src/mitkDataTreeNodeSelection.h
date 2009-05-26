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

#include <cherryISelection.h>
#include <mitkDataTreeNode.h>
#include "mitkGUICommonPluginDll.h"


namespace mitk {

class MITK_GUI_COMMON_PLUGIN DataTreeNodeSelection : public virtual cherry::ISelection
{

public:

  cherryObjectMacro(DataTreeNodeSelection);

  typedef std::vector<DataTreeNode::Pointer> NodeContainer;

  DataTreeNodeSelection();
  DataTreeNodeSelection(const NodeContainer& nodes);

  virtual const std::vector<DataTreeNode::Pointer>& GetDataTreeNodes() const;
  virtual unsigned int GetSize() const;

  /**
   * @see cherry::ISelection::IsEmpty()
   */
  bool IsEmpty() const;

protected:

  NodeContainer m_Nodes;

};

}


#endif /* MITKDATATREENODESELECTION_H_ */
