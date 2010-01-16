#ifndef QmitkDataStorageComboBox_h
#define QmitkDataStorageComboBox_h

// Own Includes
#include "mitkDataStorage.h"
#include "mitkDataTreeNode.h"
#include "mitkWeakPointer.h"
#include "mitkNodePredicateBase.h"

// Toolkit Includes
#include <QComboBox>
#include <map>

// Forward Declartions

///
/// \class QmitkDataStorageComboBox
/// \author Michael Mueller
/// \version 4.0
/// \date 2009-02-09
/// \ingroup Widgets
/// \brief Displays all or a subset (defined by a predicate) of nodes of the Data Storage.
///
/// Dont forget that this class inherits from QComboBox and you can therefore use the whole API of QComboBox.
///
class QMITK_EXPORT QmitkDataStorageComboBox : public QComboBox
{
  //#CLASS-MACROS,FRIENDS
  Q_OBJECT

  //#CTORS/DTOR
  public:    
    ///
    /// \brief Ctor for an empty combobox. Use setDataStorage and setPredicate afterwards.
    ///
    QmitkDataStorageComboBox(QWidget* parent = 0, bool _AutoSelectNewNodes=false);

    ///
    /// \brief Ctor for constructing QmitkDataStorageComboBox with given DataStorageComboBox and given _Predicate.
    ///
    QmitkDataStorageComboBox( mitk::DataStorage* _DataStorage, const mitk::NodePredicateBase* _Predicate, QWidget* parent = 0, bool _AutoSelectNewNodes=false);

    ///
    /// \brief Standard Dtor. Nothing to do here.
    ///
    ~QmitkDataStorageComboBox();

  //#PUBLIC GETTER  
  public:
    ///
    /// \brief Get the DataStorage this ComboBox listens to.
    ///
    mitk::DataStorage::Pointer GetDataStorage() const;    
    ///
    /// \brief Return the predicate (may be NULL) that is responsible for the _DataTreeNode selection of this ComboBox.
    ///
    const mitk::NodePredicateBase::ConstPointer GetPredicate() const;     
    ///
    /// \brief Returns the _DataTreeNode at Index index or 0 if the index is out of bounds.
    ///
    mitk::DataTreeNode::Pointer GetNode(int index) const;   
    ///
    /// \brief Returns the selected _DataTreeNode or 0 if there is none.
    ///
    mitk::DataTreeNode::Pointer GetSelectedNode() const;     
    ///
    /// \brief Returns all nodes that are stored in this combobox.
    ///
    mitk::DataStorage::SetOfObjects::ConstPointer GetNodes() const;   
    ///
    /// Returns the AutoSelectNewItems.
    /// \see SetAutoSelectNewItems
    ///
    virtual bool GetAutoSelectNewItems();

  //#PUBLIC SETTER
  public:
    ///
    /// \brief Set the DataStorage this ComboBox should listen to.
    ///
    /// If DataStorage is 0 nothing will be shown. If DataStorage is re-set the combobox will be resetted.
    void SetDataStorage(mitk::DataStorage* dataStorage);
    ///
    /// \brief Set the predicate for this ComboBox. (QmitkDataStorageComboBox is now owner of the predicate)
    ///
    /// If predicate is NULL all nodes will be selected. If predicate changes the whole combobox will be resetted.
    void SetPredicate(const mitk::NodePredicateBase* _Predicate); 
    ///
    /// Adds a node to the ComboBox. Gets called everytime a DataStorage Add Event was thrown.
    ///
    virtual void AddNode(const mitk::DataTreeNode* _DataTreeNode);
    ///
    /// Removes a node from the ComboBox at a specified index (if the index exists). Gets called when a DataStorage Remove Event was thrown.
    ///
    virtual void RemoveNode(int index);
    ///
    /// Removes a node from the ComboBox. Gets called when a DataStorage Remove Event was thrown.
    ///
    virtual void RemoveNode(const mitk::DataTreeNode* _DataTreeNode);
    ///
    /// Set a _DataTreeNode in the ComboBox at the specified index (if the index exists).    
    /// Internally the method just calls RemoveNode(unsigned int)
    ///
    virtual void SetNode(int index, const mitk::DataTreeNode* _DataTreeNode);
    ///
    /// Replaces a _DataTreeNode in the combobox by an _OtherDataTreeNode.
    /// Internally the method just calls SetNode(unsigned int, mitk::DataTreeNode*)
    ///
    virtual void SetNode(const mitk::DataTreeNode* _DataTreeNode, const mitk::DataTreeNode* _OtherDataTreeNode);
    ///
    /// Sets AutoSelectNewItems flag. If set to true new Nodes will be automatically selected. Default is false.
    ///
    virtual void SetAutoSelectNewItems(bool _AutoSelectNewItems);
    ///
    /// \brief Called when a node is deleted or the name property of the node was modified. Calls RemoveNode or SetNode then.
    ///
    virtual void OnDataTreeNodeDeleteOrModified(const itk::Object *caller, const itk::EventObject &event);

   

  signals:
    ///
    /// \brief Throw a signal when the _DataTreeNode selection changed.
    ///
    void OnSelectionChanged(const mitk::DataTreeNode*);

    
  //#PROTECTED GETTER
  protected:
    ///
    /// \brief Checks if the given index is within the range of the m_Nodes vector.
    ///
    bool HasIndex(unsigned int index) const;

    ///
    /// \brief Seaches for a given node and returns a valid index or -1 if the node was not found.
    ///
    int Find( const mitk::DataTreeNode* _DataTreeNode ) const;

  //#PROTECTED SETTER
  protected slots:
    ///
    /// \brief Slot for signal when the user selects another item.
    ///
    void OnCurrentIndexChanged(int);

  //#PUBLIC SETTER
  public slots:
    ///
    /// \brief Slot for signal when user wants to set a node as current selected node.
    ///
    void SetSelectedNode(mitk::DataTreeNode::Pointer item);

  protected:
    ///
    /// \brief Inserts a new node at the given index. If the index does not exist, the _DataTreeNode is simply appended to the combobox.
    ///
    /// This function is used by AddNode() and SetNode() because they just to the same:
    /// 1. If node is replaced (that is when index exists),
    ///    the itk::Event observer will be removed
    /// 2. Check Node against Predicate
    /// 3. Register for itk::Events on the node
    /// 4. Insert Node and show in combobox
    virtual void InsertNode(int index, const mitk::DataTreeNode* _DataTreeNode);

    ///
    /// \brief Init-function this class with the given dataStorage and _Predicate. This function is called by all ctors.
    ///
    void Init();

    ///
    /// \brief Reset function whenever datastorage or predicate changes.
    ///
    void Reset();

  protected:
    //#PROTECTED MEMBER VARS
    ///
    /// Pointer to the DataStorage from which the nodes are selected (remember: in BlueBerry there
    /// might be more than one DataStorage).
    ///
    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

    ///
    /// \brief Holds the predicate that is responsible for the _DataTreeNode selection of this ComboBox.
    /// If the predicate is 0, every _DataTreeNode will be selected.
    ///
    mitk::NodePredicateBase::ConstPointer m_Predicate;

    ///
    /// Holds all selected Nodes. Dont hold smart pointer as we are in a GUI class.
    ///
    std::vector<mitk::DataTreeNode*> m_Nodes;

    ///
    /// \brief Holds the tags of the node-modified observers. (must be updated everytime m_Nodes changes)
    ///
    std::vector<long> m_NodesModifiedObserverTags;

    ///
    /// \brief Holds the tags of the node-modified observers. (must be updated everytime m_Nodes changes)
    ///
    std::vector<long> m_NodesDeleteObserverTags;

    ///
    /// \brief Maps a a specific node to (Name-)property. This is needed because we have to find the assiociated node
    /// whenever the name property of a node changed.
    ///
    std::map<mitk::DataTreeNode*, const mitk::BaseProperty*> m_PropertyToNode;

    ///
    /// \brief Event function guard. Each function which is called by an event mechanism
    /// first checks if this is true in order to avoid endless loops.
    bool m_BlockEvents;

    ///
    /// \brief If set to "true" new Nodes will be automatically selected.
    bool m_AutoSelectNewNodes;

};

#endif // QmitkDataStorageComboBox_h

