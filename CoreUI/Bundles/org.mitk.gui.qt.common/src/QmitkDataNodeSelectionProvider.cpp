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

#include "QmitkDataNodeSelectionProvider.h"
#include "internal/QmitkDataNodeSelection.h"

QmitkDataNodeSelectionProvider::QmitkDataNodeSelectionProvider()
 : berry::QtSelectionProvider()
{

}

berry::ISelection::ConstPointer QmitkDataNodeSelectionProvider::GetSelection() const
{
  return this->GetDataNodeSelection();
}

mitk::DataNodeSelection::ConstPointer
QmitkDataNodeSelectionProvider::GetDataNodeSelection() const
{
  if (qSelectionModel)
  {
    QmitkDataNodeSelection::ConstPointer sel(new QmitkDataNodeSelection(
        qSelectionModel->selection()));
    return sel;
  }

  return QmitkDataNodeSelection::ConstPointer(new QmitkDataNodeSelection());
}

void QmitkDataNodeSelectionProvider::FireSelectionChanged(
    const QItemSelection&  /*selected*/, const QItemSelection&  /*deselected*/)
{
  berry::ISelection::ConstPointer sel(this->GetDataNodeSelection());
  berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(
      berry::ISelectionProvider::Pointer(this), sel));
  selectionEvents.selectionChanged(event);
}
