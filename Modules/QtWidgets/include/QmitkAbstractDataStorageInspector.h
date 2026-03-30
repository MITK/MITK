/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAbstractDataStorageInspector_h
#define QmitkAbstractDataStorageInspector_h

#include <QmitkModelViewSelectionConnector.h>

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

// qt
#include <QWidget>

/**
 * \brief Abstract base class for widgets that inspect the contents of a DataStorage.
 *
 * This class provides a convenient foundation for implementing widgets that present
 * a specific view onto a given mitk::DataStorage instance and allow the user to
 * inspect and select data nodes. Subclasses must implement Initialize(), GetView(),
 * SetSelectionMode(), and GetSelectionMode().
 *
 * The inspector uses a QmitkModelViewSelectionConnector internally to manage the
 * translation between data node selections and Qt model/view selections.
 *
 * \sa QmitkModelViewSelectionConnector
 * \sa QmitkDataStorageListInspector
 * \sa QmitkDataStorageTreeInspector
 * \sa mitkIDataStorageInspectorProvider
 */
class MITKQTWIDGETS_EXPORT QmitkAbstractDataStorageInspector : public QWidget
{
  Q_OBJECT

public:

  ~QmitkAbstractDataStorageInspector() override;

  /**
   * \brief Sets the data storage that will be used and monitored by this widget.
   *
   * If the new data storage differs from the current one and is not null,
   * the widget is re-initialized via Initialize().
   *
   * \param[in] dataStorage Pointer to the data storage to set. May be nullptr.
   */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
   * \brief Sets the node predicate and re-initializes the widget accordingly.
   *
   * The predicate is used to filter the nodes shown in the inspector view.
   * If the predicate changes, the widget is re-initialized via Initialize().
   *
   * \param[in] nodePredicate Pointer to the node predicate. May be nullptr to show all nodes.
   */
  virtual void SetNodePredicate(const mitk::NodePredicateBase* nodePredicate);

  /**
   * \brief Returns the currently set node predicate.
   * \return Pointer to the current node predicate, or nullptr if none is set.
   */
  const mitk::NodePredicateBase* GetNodePredicate() const;

  /** \brief Convenience type alias for a list of data node pointers. */
  using NodeList = QList<mitk::DataNode::Pointer>;

  /**
   * \brief Returns the list of currently selected data nodes.
   * \return A NodeList containing the currently selected nodes.
   */
  NodeList GetSelectedNodes() const;

  /**
   * \brief Returns a pointer to the view used to display the data storage content.
   * \return Pointer to the internal QAbstractItemView.
   */
  virtual QAbstractItemView* GetView() = 0;

  /** \copydoc GetView() */
  virtual const QAbstractItemView* GetView() const = 0;

  /**
   * \brief Returns whether only visible nodes can be selected.
   *
   * This reflects the setting of the internal QmitkModelViewSelectionConnector.
   * The value can be changed via SetSelectOnlyVisibleNodes().
   *
   * \return True if only visible nodes are selectable, false otherwise.
   * \sa SetSelectOnlyVisibleNodes
   */
  bool GetSelectOnlyVisibleNodes() const;

  /** \brief Convenience type alias for the Qt selection mode. */
  using SelectionMode = QAbstractItemView::SelectionMode;

  /**
   * \brief Sets the selection mode of the inspector's item view.
   * \param[in] mode The desired QAbstractItemView::SelectionMode.
   */
  virtual void SetSelectionMode(SelectionMode mode) = 0;

  /**
   * \brief Returns the current selection mode of the inspector's item view.
   * \return The active QAbstractItemView::SelectionMode.
   */
  virtual SelectionMode GetSelectionMode() const = 0;

Q_SIGNALS:
  /**
   * \brief Emitted when the set of selected nodes has changed.
   * \param[out] nodes The newly selected data nodes.
   */
  void CurrentSelectionChanged(NodeList nodes);

  public Q_SLOTS:
  /**
   * \brief Controls whether selections are restricted to visible nodes only.
   *
   * If \p selectOnlyVisibleNodes is true, an incoming selection will be filtered
   * (reduced) to only those nodes that are currently visible in the view.
   * An outgoing selection can then at most contain these visible nodes.
   *
   * If false, non-visible nodes from an incoming selection are stored internally
   * and included in outgoing selections, preserving the original selection even
   * though those nodes cannot be interactively (de-)selected in the view.
   *
   * \param[in] selectOnlyVisibleNodes If true, only visible nodes may be selected.
   * \sa GetSelectOnlyVisibleNodes
   */
  void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes);

  /**
   * \brief Sets the given list of data nodes as the new selection.
   *
   * Transforms the list of data nodes into a model selection and applies it to
   * the internal item view's selection model. The list is filtered according to
   * the current selectOnlyVisibleNodes setting. When that setting is false,
   * non-visible nodes are stored internally and merged into any outgoing
   * selection signal.
   *
   * \param[in] selectedNodes The data nodes to select.
   * \sa SetSelectOnlyVisibleNodes
   */
  void SetCurrentSelection(NodeList selectedNodes);

protected Q_SLOTS:
  void OnSelectionChanged(NodeList selectedNodes);

protected:
  /** Helper function is called if data storage or predicate is changed to (re) initialize the widget correctly.
   Implement the function in derived classes.*/
  virtual void Initialize() = 0;

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::ConstPointer m_NodePredicate;

  std::unique_ptr<QmitkModelViewSelectionConnector> m_Connector;

  QmitkAbstractDataStorageInspector(QWidget* parent = nullptr);

};

#endif
