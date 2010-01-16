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

#include "QmitkDataTreeNodeSelectionProvider.h"
#include "internal/QmitkDataTreeNodeSelection.h"

QmitkDataTreeNodeSelectionProvider::QmitkDataTreeNodeSelectionProvider()
 : berry::QtSelectionProvider()
{

}

berry::ISelection::ConstPointer QmitkDataTreeNodeSelectionProvider::GetSelection() const
{
  return this->GetDataTreeNodeSelection();
}

mitk::DataTreeNodeSelection::ConstPointer
QmitkDataTreeNodeSelectionProvider::GetDataTreeNodeSelection() const
{
  if (qSelectionModel)
  {
    QmitkDataTreeNodeSelection::ConstPointer sel(new QmitkDataTreeNodeSelection(
        qSelectionModel->selection()));
    return sel;
  }

  return QmitkDataTreeNodeSelection::ConstPointer(new QmitkDataTreeNodeSelection());
}

void QmitkDataTreeNodeSelectionProvider::FireSelectionChanged(
    const QItemSelection&  /*selected*/, const QItemSelection&  /*deselected*/)
{
  berry::ISelection::ConstPointer sel(this->GetDataTreeNodeSelection());
  berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(
      berry::ISelectionProvider::Pointer(this), sel));
  selectionEvents.selectionChanged(event);
}
