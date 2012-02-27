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

#include "QmitkDataNodeSelection.h"

#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

#include <mitkDataNodeObject.h>

QmitkDataNodeSelection::QmitkDataNodeSelection()
{

}

QmitkDataNodeSelection::QmitkDataNodeSelection(
    const QItemSelection& sel) :
  mitk::DataNodeSelection(), berry::QtItemSelection(sel)
{
  QModelIndexList indexes = sel.indexes();
  for (QModelIndexList::const_iterator index = indexes.constBegin(); index
      != indexes.constEnd(); ++index)
  {
    QVariant data = index->data(QmitkDataNodeRole);
    mitk::DataNode::Pointer node =
        data.value<mitk::DataNode::Pointer> ();
    if (node.IsNotNull())
    {
      berry::Object::Pointer obj(new mitk::DataNodeObject(node));
      mitk::DataNodeSelection::m_Selection->push_back(obj);
    }
  }
}

berry::Object::Pointer QmitkDataNodeSelection::GetFirstElement() const
{
  return mitk::DataNodeSelection::GetFirstElement();
}

QmitkDataNodeSelection::iterator QmitkDataNodeSelection::Begin() const
{
  return mitk::DataNodeSelection::Begin();
}

QmitkDataNodeSelection::iterator QmitkDataNodeSelection::End() const
{
  return mitk::DataNodeSelection::End();
}

int QmitkDataNodeSelection::Size() const
{
  return mitk::DataNodeSelection::Size();
}

QmitkDataNodeSelection::ContainerType::Pointer QmitkDataNodeSelection::ToVector() const
{
  return mitk::DataNodeSelection::ToVector();
}

bool QmitkDataNodeSelection::IsEmpty() const
{
  return mitk::DataNodeSelection::IsEmpty();
}

bool QmitkDataNodeSelection::operator==(const berry::Object* obj) const
{
  return mitk::DataNodeSelection::operator==(obj);
}
