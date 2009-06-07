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

#include "QmitkDataTreeNodeSelection.h"

#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

#include <mitkDataTreeNodeObject.h>

QmitkDataTreeNodeSelection::QmitkDataTreeNodeSelection()
{

}

QmitkDataTreeNodeSelection::QmitkDataTreeNodeSelection(
    const QItemSelection& sel) :
  mitk::DataTreeNodeSelection(), cherry::QtItemSelection(sel)
{
  QModelIndexList indexes = sel.indexes();
  for (QModelIndexList::const_iterator index = indexes.constBegin(); index
      != indexes.constEnd(); ++index)
  {
    QVariant data = index->data(QmitkDataTreeNodeRole);
    mitk::DataTreeNode::Pointer node =
        data.value<mitk::DataTreeNode::Pointer> ();
    if (node.IsNotNull())
    {
      cherry::Object::Pointer obj(new mitk::DataTreeNodeObject(node));
      mitk::DataTreeNodeSelection::m_Selection->push_back(obj);
    }
  }
}

cherry::Object::Pointer QmitkDataTreeNodeSelection::GetFirstElement() const
{
  return mitk::DataTreeNodeSelection::GetFirstElement();
}

QmitkDataTreeNodeSelection::iterator QmitkDataTreeNodeSelection::Begin() const
{
  return mitk::DataTreeNodeSelection::Begin();
}

QmitkDataTreeNodeSelection::iterator QmitkDataTreeNodeSelection::End() const
{
  return mitk::DataTreeNodeSelection::End();
}

int QmitkDataTreeNodeSelection::Size() const
{
  return mitk::DataTreeNodeSelection::Size();
}

QmitkDataTreeNodeSelection::ContainerType::Pointer QmitkDataTreeNodeSelection::ToVector() const
{
  return mitk::DataTreeNodeSelection::ToVector();
}

bool QmitkDataTreeNodeSelection::IsEmpty() const
{
  return mitk::DataTreeNodeSelection::IsEmpty();
}

bool QmitkDataTreeNodeSelection::operator==(const cherry::Object* obj) const
{
  return mitk::DataTreeNodeSelection::operator==(obj);
}
