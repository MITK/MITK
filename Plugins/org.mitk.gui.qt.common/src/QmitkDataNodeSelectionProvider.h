/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkDataNodeSelectionProvider_h
#define QmitkDataNodeSelectionProvider_h

#include <org_mitk_gui_qt_common_Export.h>

#include <berryQtSelectionProvider.h>
#include <mitkDataNodeSelection.h>

/**
 * \ingroup org_mitk_gui_qt_common
 *
 * \brief A BlueBerry selection provider for mitk::DataNode selections.
 *
 * This class works together with a Qt item selection model which holds the actual
 * selection. The underlying Qt model must support the data role \e QmitkDataNodeRole
 * and return a mitk::DataNode::Pointer object for each QModelIndex in the selection.
 *
 * You can set a Qt item selection model using QtSelectionProvider::SetItemSelectionModel().
 */
class MITK_QT_COMMON QmitkDataNodeSelectionProvider : public berry::QtSelectionProvider
{
public:

  berryObjectMacro(QmitkDataNodeSelectionProvider);

  QmitkDataNodeSelectionProvider();

  /**
   * This method will always return a mitk::DataNodeSelection object.
   */
  berry::ISelection::ConstPointer GetSelection() const override;

  using QtSelectionProvider::SetSelection;

  void SetSelection(const berry::ISelection::ConstPointer& selection, QItemSelectionModel::SelectionFlags flags) override;

protected:

  mitk::DataNodeSelection::ConstPointer GetDataNodeSelection() const;

  void FireSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
};

#endif
