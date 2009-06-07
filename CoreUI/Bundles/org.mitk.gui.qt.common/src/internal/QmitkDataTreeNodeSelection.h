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

#include <mitkDataTreeNodeSelection.h>
#include <mitkDataTreeNode.h>

#include <cherryQtItemSelection.h>

#include <vector>

class QmitkDataTreeNodeSelection: public mitk::DataTreeNodeSelection, public cherry::QtItemSelection
{
public:

  cherryObjectMacro(QmitkDataTreeNodeSelection);

  QmitkDataTreeNodeSelection();
  QmitkDataTreeNodeSelection(const QItemSelection& sel);

  virtual cherry::Object::Pointer GetFirstElement() const;
  virtual iterator Begin() const;
  virtual iterator End() const;

  virtual int Size() const;

  virtual ContainerType::Pointer ToVector() const;

  /**
   * @see cherry::ISelection::IsEmpty()
   */
  bool IsEmpty() const;

  bool operator==(const cherry::Object* obj) const;

};
