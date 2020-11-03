/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLog.h"

#include "berryQtSelectionProvider.h"

#include "berryQtItemSelection.h"
#include "berryQModelIndexObject.h"

namespace berry {

QtSelectionProvider::QtSelectionProvider() : qSelectionModel(nullptr)
{

}

void QtSelectionProvider::AddSelectionChangedListener(ISelectionChangedListener* l)
{
  selectionEvents.AddListener(l);
}

void QtSelectionProvider::RemoveSelectionChangedListener(ISelectionChangedListener* l)
{
  selectionEvents.RemoveListener(l);
}

ISelection::ConstPointer QtSelectionProvider::GetSelection() const
{
  if (qSelectionModel)
  {
    ISelection::Pointer qsel(new QtItemSelection(qSelectionModel->selection()));
    return qsel;
  }

  return ISelection::ConstPointer(nullptr);
}

void QtSelectionProvider::SetSelection(const ISelection::ConstPointer& selection)
{
  this->SetSelection(selection, QItemSelectionModel::ClearAndSelect);
}

void QtSelectionProvider::SetSelection(const ISelection::ConstPointer& selection,
                                       QItemSelectionModel::SelectionFlags flags)
{
  if (!qSelectionModel) return;

  if (QtItemSelection::ConstPointer qsel = selection.Cast<const QtItemSelection>())
  {
    qSelectionModel->select(qsel->GetQItemSelection(), flags);
  }
}

QItemSelection QtSelectionProvider::GetQItemSelection() const
{
  if (qSelectionModel)
    return qSelectionModel->selection();

  return QItemSelection();
}

void QtSelectionProvider::SetQItemSelection(const QItemSelection& selection)
{
  if (qSelectionModel)
    qSelectionModel->select(selection, QItemSelectionModel::Select);
}

QItemSelectionModel* QtSelectionProvider::GetItemSelectionModel() const
{
  return qSelectionModel;
}

void QtSelectionProvider::SetItemSelectionModel(QItemSelectionModel* selModel)
{
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

void QtSelectionProvider::FireSelectionChanged(const QItemSelection&  /*selected*/, const QItemSelection&  /*deselected*/)
{
  QtItemSelection::Pointer sel(new QtItemSelection(this->GetQItemSelection()));
  SelectionChangedEvent::Pointer event(new SelectionChangedEvent(ISelectionProvider::Pointer(this), sel));
  selectionEvents.selectionChanged(event);
}

}
