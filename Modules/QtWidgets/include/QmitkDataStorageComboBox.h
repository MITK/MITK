/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageComboBox_h
#define QmitkDataStorageComboBox_h

#include <MitkQtWidgetsExports.h>

// Own Includes
#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"
#include "mitkWeakPointer.h"

// Toolkit Includes
#include <QComboBox>
#include <map>

///
/// \ingroup QmitkModule
/// \class QmitkDataStorageComboBox
/// \author Michael Mueller
/// \version 4.0
/// \date 2009-02-09
/// \ingroup Widgets
/// \brief Displays all or a subset (defined by a predicate) of nodes of the Data Storage.
///
///
class MITKQTWIDGETS_EXPORT QmitkDataStorageComboBox : public QComboBox
{
  Q_OBJECT

public:
  ///
  /// \brief Ctor for an empty combobox. Use setDataStorage and setPredicate afterwards.
  ///
  QmitkDataStorageComboBox(QWidget *parent = nullptr, bool autoSelectNewNodes = false);

  ///
  /// \brief Ctor for constructing QmitkDataStorageComboBox with given DataStorageComboBox and given predicate.
  ///
  QmitkDataStorageComboBox(mitk::DataStorage *dataStorage,
                           const mitk::NodePredicateBase *predicate,
                           QWidget *parent = nullptr,
                           bool autoSelectNewNodes = false);

  ///
  /// \brief Standard Dtor. Nothing to do here.
  ///
  ~QmitkDataStorageComboBox() override;

  ///
  /// \brief Searches for a given node and returns a valid index or -1 if the node was not found.
  ///
  virtual int Find(const mitk::DataNode *dataNode) const;

public:
  ///
  /// \brief Get the DataStorage this ComboBox listens to.
  ///
  mitk::DataStorage::Pointer GetDataStorage() const;
  ///
  /// \brief Return the predicate (may be nullptr) that is responsible for the dataNode selection of this ComboBox.
  ///
  const mitk::NodePredicateBase::ConstPointer GetPredicate() const;
  ///
  /// \brief Returns the dataNode at Index index or 0 if the index is out of bounds.
  ///
  virtual mitk::DataNode::Pointer GetNode(int index) const;
  ///
  /// \brief Returns the selected dataNode or 0 if there is none.
  ///
  virtual mitk::DataNode::Pointer GetSelectedNode() const;
  ///
  /// \brief Returns all nodes that are stored in this combobox.
  ///
  mitk::DataStorage::SetOfObjects::ConstPointer GetNodes() const;
  ///
  /// Returns the AutoSelectNewItems.
  /// \see SetAutoSelectNewItems
  ///
  virtual bool GetAutoSelectNewItems();

public:
  ///
  /// \brief Set the DataStorage this ComboBox should listen to.
  ///
  /// If DataStorage is 0 nothing will be shown. If DataStorage is reset the combobox will be reset.
  void SetDataStorage(mitk::DataStorage *dataStorage);
  ///
  /// \brief Set the predicate for this ComboBox. (QmitkDataStorageComboBox is now owner of the predicate)
  ///
  /// If predicate is nullptr all nodes will be selected. If predicate changes the whole combobox will be reset.
  void SetPredicate(const mitk::NodePredicateBase *predicate);
  ///
  /// Adds a node to the ComboBox. Gets called every time a DataStorage Add Event was thrown.
  ///
  virtual void AddNode(const mitk::DataNode *dataNode);
  ///
  /// Removes a node from the ComboBox at a specified index (if the index exists). Gets called when a DataStorage Remove
  /// Event was thrown.
  ///
  virtual void RemoveNode(int index);
  ///
  /// Removes a node from the ComboBox. Gets called when a DataStorage Remove Event was thrown.
  ///
  virtual void RemoveNode(const mitk::DataNode *dataNode);
  ///
  /// Set a dataNode in the ComboBox at the specified index (if the index exists).
  /// Internally the method just calls RemoveNode(unsigned int)
  ///
  virtual void SetNode(int index, const mitk::DataNode *dataNode);
  ///
  /// Replaces a dataNode in the combobox by an otherDataNode.
  /// Internally the method just calls SetNode(unsigned int, mitk::DataNode*)
  ///
  virtual void SetNode(const mitk::DataNode *dataNode, const mitk::DataNode *otherDataNode);
  ///
  /// Sets AutoSelectNewItems flag. If set to true new Nodes will be automatically selected. Default is false.
  ///
  virtual void SetAutoSelectNewItems(bool autoSelectNewItems);
  ///
  /// \brief Called when the name property of the node was modified.
  ///
  virtual void OnPropertyListChanged(const itk::Object *caller, const itk::EventObject &event);

signals:
  ///
  /// \brief Throw a signal when the data node selection changed.
  ///
  void OnSelectionChanged(const mitk::DataNode *);

protected:
  ///
  /// \brief Checks if the given index is within the range of the m_Nodes vector.
  ///
  bool HasIndex(unsigned int index) const;

protected slots:
  ///
  /// \brief Slot for signal when the user selects another item.
  ///
  void OnCurrentIndexChanged(int);

public slots:
  ///
  /// \brief Slot for signal when user wants to set a node as current selected node.
  ///
  void SetSelectedNode(const mitk::DataNode::Pointer& node);

protected:
  ///
  /// \brief Inserts a new node at the given index. If the index does not exist,
  /// the data node is simply appended to the combobox.
  ///
  /// This function is used by AddNode() and SetNode() because they just to the same:
  /// 1. If node is replaced (that is when index exists),
  ///    the itk::Event observer will be removed
  /// 2. Check Node against Predicate
  /// 3. Register for itk::Events on the node
  /// 4. Insert Node and show in combobox
  virtual void InsertNode(int index, const mitk::DataNode *dataNode);

  ///
  /// \brief Init-function this class with the given data storage and predicate. This function is called by all ctors.
  ///
  void Init();

  ///
  /// \brief Reset function whenever data storage or predicate changes.
  ///
  virtual void Reset();

  void RemoveNodeAndPropertyLists(int index);

  virtual void UpdateComboBoxText(const mitk::PropertyList*);

protected:
  ///
  /// Pointer to the DataStorage from which the nodes are selected (remember: in BlueBerry there
  /// might be more than one DataStorage).
  ///
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

  ///
  /// \brief Holds the predicate that is responsible for the dataNode selection of this ComboBox.
  /// If the predicate is 0, every dataNode will be selected.
  ///
  mitk::NodePredicateBase::ConstPointer m_Predicate;

  ///
  /// Holds all selected Nodes. Don't hold smart pointer as we are in a GUI class.
  ///
  std::vector<mitk::DataNode *> m_Nodes;

  ///
  /// \brief Holds the tags of the data node property observers.
  ///
  std::vector<long> m_DataNodePropertyListObserverTags;

  ///
  /// \brief Holds the tags of the base data property observers.
  ///
  std::vector<long> m_BaseDatapropertyListObserverTags;

  ///
  /// \brief Event function guard. Each function which is called by an event mechanism
  /// first checks if this is true in order to avoid endless loops.
  bool m_BlockEvents;

  ///
  /// \brief If set to "true" new Nodes will be automatically selected.
  bool m_AutoSelectNewNodes;
};

#endif // QmitkDataStorageComboBox_h
