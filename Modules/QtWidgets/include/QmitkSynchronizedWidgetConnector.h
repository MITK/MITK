/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSynchronizedWidgetConnector_h
#define QmitkSynchronizedWidgetConnector_h

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkDataNode.h>

// mitk qt widgets
#include <QmitkSynchronizedNodeSelectionWidget.h>

// qt
#include <QList>

/*
* @brief This class connects different 'QmitkSynchronizedNodeSelectionWidget', such that
*        they can synchronize their current node selection and their current selection mode.
*
*        In order to synchronize a new node selection widget with other already connected
*        node selection widgets, 'ConnectWidget(const QmitkSynchronizedNodeSelectionWidget*)' has to be used.
*        In order to desynchronize a node selection widget,
*        'DisconnectWidget(const QmitkSynchronizedNodeSelectionWidget*)' has to be used.
*        If a new node selection has been connected / synchronized,
*        'SynchronizeWidget(QmitkSynchronizedNodeSelectionWidget*' can be used to initialy set
*        the current selection and the current selection mode.
*        For this, both values are stored in this class internally.
*/
class MITKQTWIDGETS_EXPORT QmitkSynchronizedWidgetConnector : public QObject
{
  Q_OBJECT

public:

  using NodeList = QList<mitk::DataNode::Pointer>;

  QmitkSynchronizedWidgetConnector();

  /*
  * @brief This function connects the different signals and slots of this instance and the given
  *        given node selection widget, such that changes to the current list of nodes
  *        and the selection mode can be forwarded or received.
  *        The connections are as follows:
  *           - QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged
  *             -> QmitkSynchronizedWidgetConnector::ChangeSelection
  *           - QmitkSynchronizedWidgetConnector::NodeSelectionChanged
  *             -> QmitkAbstractNodeSelectionWidget::SetCurrentSelection
  *           - QmitkSynchronizedNodeSelectionWidget::SelectionModeChanged
  *             -> QmitkSynchronizedWidgetConnector::ChangeSelectionMode
  *           - QmitkSynchronizedWidgetConnector::SelectionModeChanged
  *             -> QmitkSynchronizedNodeSelectionWidget::SetSelectAll
  *
  * @param nodeSelectionWidget    The synchronized node selection widget to be connected / synchronized.
  */
  void ConnectWidget(const QmitkSynchronizedNodeSelectionWidget* nodeSelectionWidget) const;
  /*
  * @brief This function disconnects the different signals and slot of this instance and the given
  *        given node selection widget, such that changes to the current list of nodes
  *        and the selection mode cannot be forwarded or received anymore.
  *
  * @param nodeSelectionWidget    The synchronized node selection widget to be disconnected / desynchronized.
  */
  void DisconnectWidget(const QmitkSynchronizedNodeSelectionWidget* nodeSelectionWidget) const;
  /*
  * @brief This function sets the current selection and the selection mode of the given node selection widget
  *        to the values of this instance. The required values are stored in this class internally.
  *        It can be used to newly initialize the given node selection widget.
  *
  * @param nodeSelectionWidget    The synchronized node selection widget for which the
  *                               current selection and the selection mode should be set.
  */
  void SynchronizeWidget(QmitkSynchronizedNodeSelectionWidget* nodeSelectionWidget) const;
  /*
  * @brief Get the current internal node selection.
  *
  * @return NodeList  The current internal node selection stored as a member variable.
  */
  NodeList GetNodeSelection() const;
  /*
  * @brief Get the current internal selection mode.
  *
  * @return The current internal selection mode stored as a member variable.
  */
  bool GetSelectionMode() const;

Q_SIGNALS:
  /*
  * @brief A signal that will be emitted by the 'ChangeSelection'-slot.
  *        This happens if a new selection / list of nodes is set from outside of this class,
  *        e.g. from a QmitkSynchronizedNodeSelectionWidget.
  *        This signal is connected to the 'SetCurrentSelection'-slot of each
  *        QmitkSynchronizedNodeSelectionWidget to propagate the new selection.
  *
  * @param  nodes   A list of data nodes that are newly selected.
  */
  void NodeSelectionChanged(NodeList nodes);
  /*
  * @brief A signal that will be emitted by the 'ChangeSelectionMode'-slot.
  *        This happens if the selection mode is change from outside of this class,
  *        e.g. from a QmitkSynchronizedNodeSelectionWidget.
  *        This signal is connected to the 'SetSelectAll'-slot of each
  *        QmitkSynchronizedNodeSelectionWidget to propagate the selection mode.
  *
  * @param  selectAll   True, if the selection mode is changed to "select all" nodes.
  *                     False otherwise.
  */
  void SelectionModeChanged(bool selectAll);

public Q_SLOTS:
  /*
  * @brief Set a new internal selection and send this new selection to connected
  *        QmitkSynchronizedNodeSelectionWidgets using the 'NodeSelectionChanged'-signal.
  *
  *        This slot itself is connected to the 'CurrentSelectionChanged'-signal of each
  *        QmitkSynchronizedNodeSelectionWidget to receive a new selection.
  *
  * @param  nodes   A list of data nodes that are newly selected.
  */
  void ChangeSelection(NodeList nodes);
  /*
  * @brief Set a new selection mode and send this new selection mode to connected
  *        QmitkSynchronizedNodeSelectionWidgets using the 'SelectionModeChanged'-signal.
  *
  *        This slot itself is connected to the 'SelectionModeChanged'-signal of each
  *        QmitkSynchronizedNodeSelectionWidget to receive a new selection mode.
  *
  * @param  selectAll   True, if the selection mode is changed to "select all" nodes.
  *                     False otherwise.
  */
  void ChangeSelectionMode(bool selectAll);

private:

  NodeList m_InternalSelection;
  bool m_SelectAll;

};

#endif
