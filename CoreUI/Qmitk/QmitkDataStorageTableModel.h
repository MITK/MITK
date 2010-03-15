#ifndef QmitkDataStorageTableModel_h
#define QmitkDataStorageTableModel_h

/// Own includes.
#include "mitkDataStorage.h"
#include "mitkBaseProperty.h"
#include "mitkWeakPointer.h"
#include "mitkNodePredicateBase.h"

/// Toolkit includes.
#include <QAbstractTableModel>

/// Forward declarations.

/// 
/// \class QmitkDataStorageTableModel
///
/// \brief A table model for a set of DataNodes defined by a predicate.
/// \TODO make columns interchangeable, select which properties to show as columns
///
class QMITK_EXPORT QmitkDataStorageTableModel : public QAbstractTableModel
{
  Q_OBJECT

  //#Ctors/Dtor
  public:    
    ///
    /// Constructs a new QmitkDataStorageTableModel and sets a predicate that defines
    /// this list.
    /// \see setPredicate()
    ///
    QmitkDataStorageTableModel(mitk::DataStorage::Pointer _DataStorage, mitk::NodePredicateBase* _Predicate = 0
      , QObject* parent = 0 ); 

    ///
    /// Standard dtor. Delete predicate, disconnect from DataStorage.
    ///
    virtual ~QmitkDataStorageTableModel();

  //# Public GETTER
  public:
    ///
    /// Get the DataStorage.
    /// 
    const mitk::DataStorage::Pointer GetDataStorage() const;
    ///
    /// Get the predicate.
    /// 
    mitk::NodePredicateBase::Pointer GetPredicate() const;
    ///
    /// Get node at a specific model index. Another way to implement this, is
    /// by introducing a new role like "DateTreeNode" and capture
    /// that in the data function.
    ///
    mitk::DataNode::Pointer GetNode(const QModelIndex &index) const;
    ///
    /// Overridden from QAbstractTableModel. Returns the header data at section
    /// for given orientation and role. 
    ///
    virtual QVariant headerData(int section, Qt::Orientation orientation,
      int role) const;
    ///
    /// Overridden from QAbstractTableModel. Returns what can be done
    /// with an item.
    ///
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    ///
    /// Overridden from QAbstractTableModel. Returns the node count.
    ///
    virtual int rowCount(const QModelIndex &parent) const;
    ///
    /// Overridden from QAbstractTableModel. Returns the number of features (columns) to display.
    ///
    virtual int columnCount(const QModelIndex &parent) const;
    ///
    /// Overridden from QAbstractTableModel. Returns the data at index for given role.
    ///
    virtual QVariant data(const QModelIndex &index, int role) const;

  //# Public SETTERS
  public:
    ///
    /// Sets the DataStorage.
    ///
    void SetDataStorage(mitk::DataStorage::Pointer _DataStorage);
    ///
    /// Sets the predicate. <b>QmitkDataStorageTableModel is owner of the predicate!</b>
    ///
    void SetPredicate(mitk::NodePredicateBase* _Predicate);
    ///
    /// Adds a node to this model. 
    /// There are two constraints for nodes in this model:
    /// 1. If a predicate is set (not null) the node will be checked against it.
    /// 2. The node has to have a data object (no one wants to see empty nodes).
    /// Also adds event listeners to the node.
    ///
    virtual void AddNode(const mitk::DataNode* node);
    ///
    /// Removes a node from this model. Also removes any event listener from the node.
    ///
    virtual void RemoveNode(const mitk::DataNode* node);
    ///
    /// Returns a copy of the node-vector that is shown by this model
    ///
    virtual std::vector<mitk::DataNode*> GetNodeSet() const;
    ///
    /// \brief Called when a single property was changed.
    /// The function searches through the list of nodes in this model for the changed
    /// property. If the property was found a dataChanged signal is emitted forcing
    /// all observing views to request the data again.
    ///
    virtual void PropertyModified(const itk::Object *caller, const itk::EventObject &event);
    ///
    /// Overridden from QAbstractTableModel. Sets data at index for given role.
    ///
    bool setData(const QModelIndex &index, const QVariant &value,
      int role);
    ///
    /// \brief Reimplemented sort function from QAbstractTableModel to enable sorting on the table.
    ///
    void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

    //#PROTECTED INNER CLASSES
protected:
  ///
  /// \struct DataNodeCompareFunction
  /// \brief A struct that inherits from std::binary_function. You can use it in std::sort algorithm for sorting the node list elements.
  ///
  struct DataNodeCompareFunction 
    : public std::binary_function<mitk::DataNode::Pointer, mitk::DataNode::Pointer, bool>
  {
    ///
    /// \brief Specifies field of the property with which it will be sorted.
    ///
    enum CompareCriteria {
      CompareByName = 0,
      CompareByClassName,
      CompareByVisibility
    };

    ///
    /// \brief Specifies Ascending/descending ordering.
    ///
    enum CompareOperator {
      Less = 0,
      Greater
    };

    ///
    /// \brief Creates a PropertyDataSetCompareFunction. A CompareCriteria and a CompareOperator must be given.
    ///
    DataNodeCompareFunction(CompareCriteria _CompareCriteria = CompareByName, CompareOperator _CompareOperator = Less);
    ///
    /// \brief The reimplemented compare function.
    ///
    bool operator()(const mitk::DataNode::Pointer& _Left
      , const mitk::DataNode::Pointer& _Right) const;

  protected:
    CompareCriteria m_CompareCriteria;
    CompareOperator m_CompareOperator;
  };

  //#Protected SETTER
  protected:
    ///
    /// Called when DataStorage or Predicate changed. Resets whole model and reads all nodes
    /// in again.
    ///
    virtual void Reset();

  //#Protected MEMBER VARIABLES
  protected:
    ///
    /// Pointer to the DataStorage from which the nodes are selected (remember: in BlueBerry there
    /// might be more than one DataStorage).
    /// Store it in a weak pointer. This is a GUI class which should not hold a strong reference
    /// to any non-GUI Object.
    ///
    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
    ///
    /// Holds the predicate that defines this SubSet of Nodes. If m_Predicate
    /// is NULL all Nodes will be selected.
    ///
    mitk::NodePredicateBase::Pointer m_Predicate;
    ///
    /// Holds all selected Nodes.
    ///
    std::vector<mitk::DataNode*> m_NodeSet;
    ///
    /// \brief Maps a property to an observer tag.
    ///
    std::map<mitk::BaseProperty*, unsigned long> m_NamePropertyModifiedObserverTags;
    ///
    /// \brief Maps a property to an observer tag.
    ///
    std::map<mitk::BaseProperty*, unsigned long> m_VisiblePropertyModifiedObserverTags;
    ///
    /// Saves if this model is currently working on events to prevent endless event loops.
    /// 
    bool m_BlockEvents;
    ///
    /// \brief The property is true when the property list is sorted in descending order.
    ///
    bool m_SortDescending;

};

#endif

