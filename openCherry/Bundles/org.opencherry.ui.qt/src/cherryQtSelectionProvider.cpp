/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryQtSelectionProvider.h"

#include "cherryQtItemSelection.h"

namespace cherry {

QtSelectionProvider
::QtSelectionProvider() : qSelectionModel(0)
{

}

void
QtSelectionProvider
::AddSelectionChangedListener(ISelectionChangedListener::Pointer l)
{
  selectionEvents.AddListener(l);
}

void
QtSelectionProvider
::RemoveSelectionChangedListener(ISelectionChangedListener::Pointer l)
{
  selectionEvents.RemoveListener(l);
}

 ISelection::ConstPointer
 QtSelectionProvider
::GetSelection() const
{
   if (qSelectionModel)
   {
   QtItemSelection::Pointer qsel(new QtItemSelection(qSelectionModel->selection()));
   return qsel;
   }

   return QtItemSelection::Pointer(new QtItemSelection());
}

 void
 QtSelectionProvider
::SetSelection(ISelection::Pointer selection)
 {
   if (!qSelectionModel) return;

   if (QtItemSelection::Pointer qsel = selection.Cast<QtItemSelection>())
   {
     qSelectionModel->select(qsel->GetQItemSelection(), QItemSelectionModel::Select);
   }
 }

 QItemSelection
 QtSelectionProvider
::GetQItemSelection() const
{
   if (qSelectionModel)
   return qSelectionModel->selection();

   return QItemSelection();
}

 void
 QtSelectionProvider
::SetQItemSelection(const QItemSelection& selection)
 {
   if (qSelectionModel)
     qSelectionModel->select(selection, QItemSelectionModel::Select);
 }

  QItemSelectionModel*
  QtSelectionProvider
::GetItemSelectionModel() const
{
    return qSelectionModel;
}

void QtSelectionProvider::SetItemSelectionModel(QItemSelectionModel* selModel)
{
  std::cout << "Setting item selection model for provider: " << selModel << std::endl;
  if (qSelectionModel)
  {
    qSelectionModel->disconnect(this);
  }

  qSelectionModel = selModel;

  if (qSelectionModel)
  {
    this->connect(qSelectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(FireSelectionChanged(const QItemSelection&, const QItemSelection&)));
  }
}

void QtSelectionProvider::FireSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{

  QtItemSelection::Pointer sel(new QtItemSelection(this->GetQItemSelection()));
  SelectionChangedEvent::Pointer event(new SelectionChangedEvent(ISelectionProvider::Pointer(this), sel));
  selectionEvents.selectionChanged(event);
}

}
