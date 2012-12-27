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

#include "QmitkDataNodeSelectionProvider.h"

#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

#include "internal/QmitkDataNodeSelection.h"

QmitkDataNodeSelectionProvider::QmitkDataNodeSelectionProvider()
 : berry::QtSelectionProvider()
{

}

berry::ISelection::ConstPointer QmitkDataNodeSelectionProvider::GetSelection() const
{
  return this->GetDataNodeSelection();
}

void QmitkDataNodeSelectionProvider::SetSelection(berry::ISelection::ConstPointer selection,
                                                  QItemSelectionModel::SelectionFlags flags)
{
  if (!qSelectionModel) return;

  mitk::DataNodeSelection::ConstPointer dataNodeSelection = selection.Cast<const mitk::DataNodeSelection>();
  if (dataNodeSelection)
  {
    const QAbstractItemModel* model = qSelectionModel->model();

    QItemSelection newSelection;
    const std::list<mitk::DataNode::Pointer> selectedNodes = dataNodeSelection->GetSelectedDataNodes();
    for (std::list<mitk::DataNode::Pointer>::const_iterator i = selectedNodes.begin();
         i != selectedNodes.end(); ++i)
    {
      QModelIndexList matched = model->match(model->index(0, 0), QmitkDataNodeRawPointerRole,
                                             QVariant::fromValue<mitk::DataNode*>(i->GetPointer()), 1, Qt::MatchRecursive);
      if (!matched.empty())
      {
        newSelection.select(matched.front(), matched.front());
      }
    }
    qSelectionModel->select(newSelection, flags);
  }
  else
  {
    QtSelectionProvider::SetSelection(selection, flags);
  }
}

mitk::DataNodeSelection::ConstPointer
QmitkDataNodeSelectionProvider::GetDataNodeSelection() const
{
  if (qSelectionModel)
  {
    mitk::DataNodeSelection::ConstPointer sel(new QmitkDataNodeSelection(
        qSelectionModel->selection()));
    return sel;
  }

  return mitk::DataNodeSelection::ConstPointer(0);
}

void QmitkDataNodeSelectionProvider::FireSelectionChanged(
    const QItemSelection&  /*selected*/, const QItemSelection&  /*deselected*/)
{
  berry::ISelection::ConstPointer sel(this->GetDataNodeSelection());
  berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(
      berry::ISelectionProvider::Pointer(this), sel));
  selectionEvents.selectionChanged(event);
}
