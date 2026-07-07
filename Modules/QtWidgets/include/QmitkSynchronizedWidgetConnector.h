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

/**
* \brief This class connects different 'QmitkSynchronizedNodeSelectionWidget', such that
*        they can synchronize their current node selection and their current selection mode.
*
*        In order to synchronize a new node selection widget with other already connected
*        node selection widgets, 'ConnectWidget(const QmitkSynchronizedNodeSelectionWidget*)' has to be used.
*        In order to desynchronize a node selection widget,
*        'DisconnectWidget(const QmitkSynchronizedNodeSelectionWidget*)' has to be used.
*        If a new node selection has been connected / synchronized,
*        'SynchronizeWidget(QmitkSynchronizedNodeSelectionWidget*' can be used to initially set
*        the current selection and the current selection mode.
*        For this, both values are stored in this class internally.
*
* \note  Per-renderer node properties this connector fans out via per-cell writes:
*          - visible (bool) - written via QmitkSynchronizedNodeSelectionWidget::SetNodeVisibility
*                             and propagated to other widgets through the
*                             NodeVisibilityChanged signal pair on this connector.
*          - layer   (int)  - written via QmitkRenderWindowDataNodeTableModel::moveNodesLayer
*                             (which calls mitk::RenderWindowLayerController::MoveNodeToPosition)
*                             and propagated through the NodesLayerMoved signal pair.
*
*        Connector-internal state (NOT fanned out via per-renderer property writes):
*          - selection list   (m_InternalSelection)
*          - select_all mode  (m_SelectAll)
*          - invisibles set   (m_InternalInvisibles)
*
*        Selection-list membership is propagated to other widgets via the
*        NodeSelectionChanged signal, not via per-renderer writes - selection is
*        a runtime concept tracked at the widget level. select_all is group-scoped
*        and authored in the layout document; the protected SeedFromMember entry
*        deliberately does NOT touch it.
*/
class MITKQTWIDGETS_EXPORT QmitkSynchronizedWidgetConnector : public QObject
{
  Q_OBJECT

public:

  using NodeList = QmitkSynchronizedNodeSelectionWidget::NodeList;

  QmitkSynchronizedWidgetConnector();

  /**
  * \brief This function connects the different signals and slots of this instance and the given
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
  * \param nodeSelectionWidget    The synchronized node selection widget to be connected / synchronized.
  */
  void ConnectWidget(const QmitkSynchronizedNodeSelectionWidget* nodeSelectionWidget);
  /**
  * \brief This function disconnects the different signals and slot of this instance and the given
  *        given node selection widget, such that changes to the current list of nodes
  *        and the selection mode cannot be forwarded or received anymore.
  *
  * \param nodeSelectionWidget    The synchronized node selection widget to be disconnected / desynchronized.
  */
  void DisconnectWidget(const QmitkSynchronizedNodeSelectionWidget* nodeSelectionWidget);
  /**
  * \brief This function sets the current selection and the selection mode of the given node selection widget
  *        to the values of this instance. The required values are stored in this class internally.
  *        It can be used to newly initialize the given node selection widget.
  *
  * \param nodeSelectionWidget    The synchronized node selection widget for which the
  *                               current selection and the selection mode should be set.
  */
  void SynchronizeWidget(QmitkSynchronizedNodeSelectionWidget* nodeSelectionWidget) const;
  /**
  * \brief Get the current internal node selection.
  *
  * \return NodeList  The current internal node selection stored as a member variable.
  */
  NodeList GetNodeSelection() const;

  /**
  * \brief Get the current internal selection mode.
  *
  * \return The current internal selection mode stored as a member variable.
  */
  bool GetSelectionMode() const;

Q_SIGNALS:
  /**
  * \brief A signal that will be emitted by the 'ChangeSelection'-slot.
  *        This happens if a new selection / list of nodes is set from outside of this class,
  *        e.g. from a QmitkSynchronizedNodeSelectionWidget.
  *        This signal is connected to the 'SetCurrentSelection'-slot of each
  *        QmitkSynchronizedNodeSelectionWidget to propagate the new selection.
  *
  * \param  nodes   A list of data nodes that are newly selected.
  */
  void NodeSelectionChanged(NodeList nodes);
  /**
  * \brief A signal that will be emitted by the 'ChangeSelectionMode'-slot.
  *        This happens if the selection mode is change from outside of this class,
  *        e.g. from a QmitkSynchronizedNodeSelectionWidget.
  *        This signal is connected to the 'SetSelectAll'-slot of each
  *        QmitkSynchronizedNodeSelectionWidget to propagate the selection mode.
  *
  * \param  selectAll   True, if the selection mode is changed to "select all" nodes.
  *                     False otherwise.
  */
  void SelectionModeChanged(bool selectAll);

  void NodeVisibilityChanged(mitk::DataNode::Pointer node, const bool visibility);
  void NodesLayerMoved(QSet<mitk::DataNode*> movedNodes, const int targetLayer);

public Q_SLOTS:
  /**
  * \brief Set a new internal selection and send this new selection to connected
  *        QmitkSynchronizedNodeSelectionWidgets using the 'NodeSelectionChanged'-signal.
  *
  *        This slot itself is connected to the 'CurrentSelectionChanged'-signal of each
  *        QmitkSynchronizedNodeSelectionWidget to receive a new selection.
  *
  * \param  nodes   A list of data nodes that are newly selected.
  */
  void ChangeSelection(NodeList nodes);

  /**
  * \brief Set a new selection mode and send this new selection mode to connected
  *        QmitkSynchronizedNodeSelectionWidgets using the 'SelectionModeChanged'-signal.
  *
  *        This slot itself is connected to the 'SelectionModeChanged'-signal of each
  *        QmitkSynchronizedNodeSelectionWidget to receive a new selection mode.
  *
  * \param  selectAll   True, if the selection mode is changed to "select all" nodes.
  *                     False otherwise.
  */
  void ChangeSelectionMode(bool selectAll);

  /**
  * \brief When the visibility of a node is changed in a connected widget,
  *        update the internal set of invisible selected nodes.
  *        This information is needed to fully synchronize newly connected widgets.
  * \param  node   A pointer to the data node for which visibility was toggled.
  * \param  visibility   The new visibility of the respective data node.
  */
  void OnNodeVisibilityChanged(mitk::DataNode::Pointer node, bool visibility);

  /**
  * \brief Decrease the internal counter of connections to keep track of how many
  *        QmitkSynchronizedNodeSelectionWidgets are synchronized.
  *
  *        This slot itself is connected to the 'DeregisterSynchronization'-signal of each
  *        QmitkSynchronizedNodeSelectionWidget to get notified when a synchronized
  *        widget is deleted.
  */
  void DeregisterWidget();

protected:

  /**
  * \brief Seed this connector's runtime selection from a designated member cell.
  *
  *        Used by the v2 layout applier to make a group's seed cell authoritative
  *        for the connector's `m_InternalSelection` and `m_InternalInvisibles`.
  *        The seed cell is the cell that appears first in document order whose
  *        `links.selection` references this group (see the schema description
  *        of the seeding rule).
  *
  *        The connector takes the seed cell's selection list verbatim and
  *        rebuilds its invisible-nodes set by reading per-renderer visibility
  *        from `seedRenderer` for each selected node.
  *
  *        Group-scoped state is not touched: `m_SelectAll` is set from the
  *        layout document's `groups` dict, not derived from any cell, so
  *        SeedFromMember leaves it alone.
  *
  *        This method is intended for layout-load seeding only. Calling it
  *        outside of `ApplyLayout`'s seeding pass risks clobbering the
  *        runtime state that the user has built up since load time. Access
  *        is restricted to `QmitkMxNMultiWidget` (via friendship) and to
  *        test fixtures that subclass this type to exercise the contract
  *        directly.
  *
  * \param seedSelection  The selection list of the seed cell.
  * \param seedRenderer   The base renderer of the seed cell. Per-node
  *                       visibility is read from this renderer to rebuild
  *                       `m_InternalInvisibles`. Must not be null.
  */
  void SeedFromMember(const NodeList& seedSelection, const mitk::BaseRenderer* seedRenderer);

private:

  friend class QmitkMxNMultiWidget;

  NodeList m_InternalSelection;
  std::set<mitk::DataNode*> m_InternalInvisibles;
  bool m_SelectAll;
  unsigned int m_ConnectionCounter;

};

#endif
