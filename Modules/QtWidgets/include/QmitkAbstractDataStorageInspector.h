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
* @brief This abstract class is a convenient base class for easy implementation of widgets that
* offer a specific view onto a given DataStorage instance to inspect its contents.
* One may also get the selection in this inspector of the data storage.
*/
class MITKQTWIDGETS_EXPORT QmitkAbstractDataStorageInspector : public QWidget
{
  Q_OBJECT

public:

  ~QmitkAbstractDataStorageInspector() override;

  /**
  * @brief Sets the data storage that will be used /monitored by the widget.
  *
  * @param dataStorage      A pointer to the data storage to set.
  */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
  * @brief Sets the node predicate and updates the widget, according to the node predicate.
  *
  * @param nodePredicate    A pointer to node predicate.
  */
  virtual void SetNodePredicate(const mitk::NodePredicateBase* nodePredicate);

  const mitk::NodePredicateBase* GetNodePredicate() const;

  using NodeList = QList<mitk::DataNode::Pointer>;

  /** Returns the list of currently selected nodes.*/
  NodeList GetSelectedNodes() const;

  /** Returns an pointer to the view that is used in the inspector to show the content.*/
  virtual QAbstractItemView* GetView() = 0;
  virtual const QAbstractItemView* GetView() const = 0;

  /** Returns the setting of the internal connector. It can be changed by SetSelectOnlyVisibleNodes()*/
  bool GetSelectOnlyVisibleNodes() const;

  using SelectionMode = QAbstractItemView::SelectionMode;

  /** Sets the selection mode of the inspector.*/
  virtual void SetSelectionMode(SelectionMode mode) = 0;
  virtual SelectionMode GetSelectionMode() const = 0;

Q_SIGNALS:
  /**
  * @brief A signal that will be emitted if the selected node has changed.
  *
  * @param nodes		A list of data nodes that are newly selected.
  */
  void CurrentSelectionChanged(NodeList nodes);

  public Q_SLOTS:
  /**
  * @brief Change the selection modus of the item view's selection model.
  *
  *   If true, an incoming selection will be filtered (reduced) to only those nodes that are visible by the current view.
  *   An outgoing selection can then at most contain the filtered nodes.
  *   If false, the incoming non-visible selection will be stored and later added to the outgoing selection,
  *   to include the original selection that could not be modified.
  *   The part of the original selection, that is non-visible are the nodes that are not
  *
  * @param selectOnlyVisibleNodes   The bool value to define the selection modus.
  */
  void SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes);

  /**
  * @brief Transform a list of data nodes into a model selection and set this as a new selection of the
  *        selection model of the private member item view.
  *
  *   The function filters the given list of nodes according to the 'm_SelectOnlyVisibleNodes' member variable. If
  *   necessary, the non-visible nodes are stored. This is done if 'm_SelectOnlyVisibleNodes' is false: In this case
  *   the selection may be filtered and only a subset of the selected nodes may be visible and therefore (de-)selectable
  *   in the data storage viewer. By storing the non-visible nodes it is possible to send the new, modified selection
  *   but also include the selected nodes from the original selection that could not be modified (see 'SetSelectOnlyVisibleNodes').
  *
  * @param selectedNodes A list of data nodes that should be newly selected.
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
