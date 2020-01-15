/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

void QmitkDataNodeSelectionProvider::SetSelection(const berry::ISelection::ConstPointer& selection,
                                                  QItemSelectionModel::SelectionFlags flags)
{
  if (!qSelectionModel) return;

  mitk::DataNodeSelection::ConstPointer dataNodeSelection = selection.Cast<const mitk::DataNodeSelection>();
  if (dataNodeSelection)
  {
    const QAbstractItemModel* model = qSelectionModel->model();

    QItemSelection newSelection;
    const std::list<mitk::DataNode::Pointer> selectedNodes = dataNodeSelection->GetSelectedDataNodes();
    for (auto i = selectedNodes.begin();
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

  return mitk::DataNodeSelection::ConstPointer(nullptr);
}

void QmitkDataNodeSelectionProvider::FireSelectionChanged(
    const QItemSelection&  /*selected*/, const QItemSelection&  /*deselected*/)
{
  berry::ISelection::ConstPointer sel(this->GetDataNodeSelection());
  berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(
      berry::ISelectionProvider::Pointer(this), sel));
  selectionEvents.selectionChanged(event);
}
