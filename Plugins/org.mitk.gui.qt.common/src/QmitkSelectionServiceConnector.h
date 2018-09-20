/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKSELECTIONSERVICECONNECTOR_H
#define QMITKSELECTIONSERVICECONNECTOR_H

#include <org_mitk_gui_qt_common_Export.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

// mitk gui qt common plugin
#include "QmitkDataNodeSelectionProvider.h"
#include "internal/QmitkDataNodeItemModel.h"

 // blueberry ui qt plugin
#include <berryISelectionService.h>

/*
* @brief The 'QmitkSelectionServiceConnector' is used to handle the selections of the global selection bus (selection service).
*
*   The selection service connector can listen to a selection service. This should be done by using 'AddPostSelectionListener'
*   with the existing selection service of the surrounding 'QmitkAbstractView'.
*   The selection service connector can provide selections. This should be done by using 'SetAsSelectionProvider'
*   with the existing selection provider of the surrounding 'QmitkAbstractView'.
*
*   The 'QmitkSelectionServiceConnector' offers a public slot and signal that can be used to propagate the selected
*   nodes from or to the global selection bus:
*   The 'ChangeServiceSelection'-slot transforms the given list of selected nodes into a QItemSelection of a temporary
*   data node selection model. This data node selection model is set as the item selection model of the member selection provider.
*   So by temporary adding a new data node selection model and changing its selection the selection provider sends a new selection
*   that can be received at any place in the workbench.
*
*   The 'ServiceSelectionChanged'-signal sends a list of selected nodes to it's local environment (e.g. containing widget).
*   The 'ServiceSelectionChanged'-signal is emitted by the 'ServiceSelectionChanged'-function, which transforms the
*   berry selection of the selection  into a data node list. The 'ServiceSelectionChanged'-function is called whenever
*   the selection service sends a selection changed event.
*
*   In order to connect the 'QmitkSelectionServiceConnector' with a model-view pair, a 'QmitkModelViewSelectionConnector' needs to be used:
*   The 'QmitkModelViewSelectionConnector' offers a 'SetCurrentSelection'-slot that can be connected with the
*   'ServiceSelectionChanged'-signal of this class.
*   The 'QmitkModelViewSelectionConnector' offers a 'CurrentSelectionChanged'-signal that can be connected with the
*   'ChangeServiceSelection'-slot of this class.
*/
class MITK_QT_COMMON QmitkSelectionServiceConnector : public QObject
{
  Q_OBJECT

public:

  QmitkSelectionServiceConnector();
  ~QmitkSelectionServiceConnector();

  /*
  * @brief 	Create a selection listener and add it to the list of selection listener of the given selection service.
  *
  *   The selection listener is connected to the 'ServiceSelectionChanged' member function, which is
  *   called if a berry selection is changed in the workbench.
  */
  void AddPostSelectionListener(berry::ISelectionService* selectionService);
  /*
  * @brief 	Remove a selection listener from the list of selection listener of the selection service member.
  */
  void RemovePostSelectionListener();
  /*
  * @brief  Store the given selection provider as a private member.
  *         In order to use the public slot 'ChangeServiceSelection'-function, the selection provider member had to be
  *         previously set.
  */
  void SetAsSelectionProvider(QmitkDataNodeSelectionProvider* selectionProvider);
  /*
  * @brief  Set the selection provider member to a nullptr. This will prevent the public slot
  *         'ChangeServiceSelection'-function from working.
  */
  void RemoveAsSelectionProvider();

Q_SIGNALS:
  /*
  * @brief A signal that will be emitted by the private 'ServiceSelectionChanged'-function. This happens if a selection is changed
  *        via the selection service.
  *
  * @par	nodes		A list of data nodes that are newly selected.
  */
  void ServiceSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  /*
  * @brief A signal that will be emitted by the private 'ServiceSelectionChanged'-function. If sourcePart has send an invalid selection
  * (selection pointer was Null).
  * @par	sourcePart		Part that sent the null selection.
  */
  void ServiceNullSelection(const berry::IWorkbenchPart::Pointer& sourcePart);

public Q_SLOTS:
  /*
  * @brief  Send new selections to the selection service via the private selection provider member.
  *
  *   This slot-function is called whenever a local selection is changed in the surrounding widget and a selection provider was set.
  *   The newly selected data nodes are added temporary to a 'QmitkDataNodeItemModel', which is then used to define
  *   the indices to select.
  *   The 'QItemSelectionModel' is set as the item selection model of the selection provider member and its items are
  *   selected by the indices previously defined by the 'QmitkDataNodeItemModel'.
  */
  void ChangeServiceSelection(QList<mitk::DataNode::Pointer> nodes);

private:

  std::unique_ptr<berry::ISelectionListener> m_BerrySelectionListener;
  berry::ISelectionService* m_SelectionService;
  QmitkDataNodeSelectionProvider* m_SelectionProvider;
  std::shared_ptr<QmitkDataNodeItemModel> m_DataNodeItemModel;
  std::shared_ptr<QItemSelectionModel> m_DataNodeSelectionModel;

  /*
  * @brief  Handle a selection received from the selection service.
  *
  *   This function is called whenever a berry selection of the selection service is changed in the workbench.
  *   The new selection is transformed into a data node selection and the contained data nodes are propagated
  *   as the new current selection of the item view member.
  *
  * @par  sourcePart  The workbench part containing the selection.
  * @par  selection   The current selection.
  */
  void OnServiceSelectionChanged(const berry::IWorkbenchPart::Pointer& sourcePart, const berry::ISelection::ConstPointer& selection);

};

#endif // QMITKSELECTIONSERVICECONNECTOR_H
