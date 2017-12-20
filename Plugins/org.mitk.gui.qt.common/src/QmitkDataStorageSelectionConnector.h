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

#ifndef QMITKDATASTORAGESELECTIONCONNECTOR_H
#define QMITKDATASTORAGESELECTIONCONNECTOR_H

#include <org_mitk_gui_qt_common_Export.h>

// qt widgets module
#include <QmitkDataStorageAbstractView.h>

// mitk gui qt common plugin
#include "QmitkDataNodeSelectionProvider.h"
#include "internal/QmitkDataNodeItemModel.h"

 // blueberry ui qt plugin
#include <berryISelectionService.h>

 /*
 * @brief The QmitkDataStorageSelectionConnector is used to connect a given 'QmitkDataStorageAbstractView' with the global selection bus.
 *        The data storage view can be added as a selection listener to a selection service. This should be done by using 'AddPostSelectionListener'
 *        with the existing selection service of the surrounding 'QmitkAbstractView'.
 *        The data storage view can be set as a selection provider. This should be done by using 'SetAsSelectionProvider' with the existing
 *        selection provider of the surrounding 'QmitkAbstractView'.
 */
class MITK_QT_COMMON QmitkDataStorageSelectionConnector : public QObject
{
  Q_OBJECT

public:

  QmitkDataStorageSelectionConnector(QmitkDataStorageAbstractView* dataStorageAbstractView);
  ~QmitkDataStorageSelectionConnector();

  /*
  * @brief 	Create a selection listener and add it to the list of selection listener of the given selection service.
  *         The selection listener is connected with the 'GlobalSelectionChanged' member function, which is
  *         called if a berry selection is changed in the workbench.
  */
  void AddPostSelectionListener(berry::ISelectionService* selectionService);
  /*
  * @brief 	Remove a selection listener from the list of selection listener of the selection service member.
  */
  void RemovePostSelectionListener();
  /*
  * @brief 	Connect the 'CurrentSelectionChanged'-slot from the member 'QmitkDataStorageAbstractView' with the
  *         private 'FireSelectionChanged'-function of this class. The 'FireSelectionChanged'-function transforms the
  *         list of selected nodes and propagates the changed selection.
  */
  void SetAsSelectionProvider(QmitkDataNodeSelectionProvider* selectionProvider);
  /*
  * @brief 	Disconnect the 'CurrentSelectionChanged'-slot of the member 'QmitkDataStorageAbstractView' from the
  *         private 'FireSelectionChanged'-function of this class.
  */
  void RemoveAsSelectionProvider();

private Q_SLOTS:

  /*
  * @brief 	Send global selections to the selection provider.
  *
  *   This function is called whenever a local selection is changed in the workbench and the 'QmitkDataStorageAbstractView'
  *   is set as selection provider.
  *   The newly selected data nodes are added temporary to the 'QmitkDataNodeItemModel', which is then used to define
  *   the indices to select.
  *   The 'QItemSelectionModel' is set for the selection provider and its items are selected by the indices previously defined.
  */
  void FireGlobalSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

private:

  QmitkDataStorageAbstractView* m_DataStorageAbstractView;
  std::unique_ptr<berry::ISelectionListener> m_BerrySelectionListener;
  berry::ISelectionService* m_SelectionService;
  QmitkDataNodeSelectionProvider* m_SelectionProvider;
  std::shared_ptr<QmitkDataNodeItemModel> m_DataNodeItemModel;
  std::shared_ptr<QItemSelectionModel> m_DataNodeSelectionModel;

  /*
  * @brief 	Handle global selection received from the selection service.
  *
  *   This function is called whenever a global berry selection is changed in the workbench.
  *   The new selection is transformed into a data node selection and the contained data nodes are propagated
  *   as the new current selection of the data storage view member.
  */
  void GlobalSelectionChanged(const berry::IWorkbenchPart::Pointer& sourcePart, const berry::ISelection::ConstPointer& selection);

};

#endif // QMITKDATASTORAGESELECTIONCONNECTOR_H
