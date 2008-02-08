/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef MITK_DATATREEFILTER_H_INCLUDED
#define MITK_DATATREEFILTER_H_INCLUDED

#include <mitkDataTree.h> 
#include <itkSmartPointerVectorContainer.h>
#include "itkMacro.h"
#include <set>
#include "mitkDataStorage.h"
/*

   TODO / Ideen:

   - Die views bekommen vom Filter nur Infos ueber hinzufuegen/loeschen von items.

   - fuer aenderungen der properties muessen sich die views selbst bei denen anmelden

     Ueber die member GetIndex() und IsRoot() GetParent() kann die View feststellen, welches
     ViewItem (Qt) gemeint ist (oder die View haelt sich da irgendeine Tabelle fuer die
     Zuordnung). GetIndex() liefert die Position unterhalb vom Parent, und wenn
     IsRoot() true liefert, dann ist man schon auf hoechster Ebene. 

   - Die Auswahl (welche Items sind selektiert) gehoert in das Modell. Damit kann man in
     mehreren Views die Auswahl konsistent halten (Combo- und Listbox).
     Die Auswahl gehoert nicht in den Datenbaum, weil man so verhindern wuerde, dass zwei
     unabhaengige Comboboxen, die dann im Hintergrund zwei versch. Filter haben, ihre
     Auswahl voneinander trennen koennen.

*/

/**
  \class mitk::DataTreeFilter mitkDataTreeFilter.h mitkDataTreeFilter.h

  \brief Provides a filtered view on the data tree, useful as basis for GUI elements (such as QmitkDataTreeListView and QmitkDataTreeComboBox).
    
  Before reading this, you should know the following classes: 
  DataTree, DataTreeNode,  itk::TreeIteratorBase, BaseProperty, itk::Event.

  Available sections:
  <ul>
  <li> \ref sectionMitkDataTreeFilterOverview
  <li> \ref sectionMitkDataTreeFilterUsage 
    <ul>
    <li> \ref sectionMitkDataTreeFilterUsage1
      <ul>
      <li> \ref sectionMitkDataTreeFilterUsage11
      <li> \ref sectionMitkDataTreeFilterUsage12
      <li> \ref sectionMitkDataTreeFilterUsage13
      <li> \ref sectionMitkDataTreeFilterUsage14
      <li> \ref sectionMitkDataTreeFilterUsage15
      </ul>
    <li> \ref sectionMitkDataTreeFilterUsage2
      <ul>
      <li> \ref sectionMitkDataTreeFilterUsage21
      <li> \ref sectionMitkDataTreeFilterUsage22
      </ul>
    </ul>
  <li> \ref sectionMitkDataTreeFilterUndocumented 
  </ul>
  
  \section sectionMitkDataTreeFilterOverview Overview 

  This class is intended to provide a ready-to-use model of the data tree,
  so that GUI classes, which want to display information about some
  nodes in the data tree, have to add only code for the UI.
  
  What does this class offer to you beyond the features of a tree iterator? 
  These are the features:

    - The offered model of the data tree is <b>constrained by a filter</b>, i.e. you
      only get tree nodes of your very special interest. The filter is realized as a
      user provided function pointer (kind of). You can define filters that suit your
      needs quite easily.
      A sensible set of default filters is available in the mitk namespace.
        
    - Not all views of the data tree need the hierarchy of it preserved. So <b>you can
      choose between a hierarchy-preserving and a flattening model</b>. You can switch
      between these possibilities without recreating the model.
        
    - Usually you want not only to display the content of the data tree, but you want to
      let the user choose some nodes of the tree. Therefore this class keeps <b>a
      selection marker</b> for each displayed node. You can
      choose between <b>single-select or multi-select</b>. 
        
    - In many cases you will want to display some properties for each node of the
      (filtered) data tree. For this end, this class holds <b>two lists to tell GUI classes
      which properties to display and which to make editable</b>. Together with the
      derivations of PropertyObserver, this allows to implement GUI classes that can 
      display a whole range of different information on tree nodes.

    - To keep clients / GUI elements in sync with the model of the data tree, the
      tree filter uses the <b>ITK event mechanism to notify about changes</b>.
      Clients can turn off the automatic synchronization with the data tree by calling
      SetAutoUpdate(false).
      

  Why would you want such features? You will like them, if ...

    - you need to display a list of images from the data tree.
    - a user should be able to select one (several) image(s) from that list.
    - you need two lists of images and a user should be able to select an image in both
      independently. This is the motivation for keeping the selection status within the
      items and not in a property of the DataTreeNodes.
    - you want to display color and name of all segmentations in your data tree.
    - you need a list of all children of an image that are also segmentations, e.g. for an 
      Interactor that wants to determine the object under the mouse cursor.
    - you want to save parts the data tree, but only nodes that actually contain data
  
  \image html doc_mitkDataTreeFilterRelatedClasses.png 
  \image latex doc_mitkDataTreeFilterRelatedClasses.eps

  The above diagram illustrates the relationship between the MITK data tree, the
  DataTreeFilter and its clients. The most important observation is, that <i>the filter does
  not destroy or modify the data tree</i>. It doesn't even change a single property of any
  tree node. The filter simply provides a representation that is more suitable to GUI classes than
  is a tree iterator. The client gets a list of so called items,
  where <i>each Item is something like a convenience pointer to a DataTreeNode</i>.
  You can ask items to return the node's properties, if you need it
  you can get the bare tree node, and you can (un)select items.
  To represent a tree structure, each item can have a list of children.

  \section sectionMitkDataTreeFilterUsage Usage examples
 
  This section will discuss some code blocks that are commonly needed when working with the
  DataTreeFilter. For the whole truth, have a look at the implementation of
  (recommended order)
  DataTreeFilter, QmitkDataTreeComboBox, and QmitkDataTreeListView.

  This section is divided into two parts: first, we describe how to setup a
  DataTreeFilter, so that it may be used by some GUI element. Then we look at the
  DataTreeFilter from the point of view of a GUI class, i.e. we consider how to access the
  information contained in the items.

  \subsection sectionMitkDataTreeFilterUsage1 Setting up a data tree filter
  
  This sections tells you how to prepare a DataTreeFilter. If you are a user of the MITK
  library, this will most probably the only thing you have to do with this class. There
  are GUI classes which will take the prepared filter and display its information, but the
  internals of how they process this information need not worry you.
  
  \subsubsection sectionMitkDataTreeFilterUsage11 Creating a data tree filter

  The minimum you have to provide in order to use the DataTreeFilter, is the data tree.
  You have to do this in the constructor, because a filter does not make sense without a
  tree (and using more than one tree is not very common).

\code
  #include <mitkDataTreeFilter.h>

  //...

  mitk::DataTreeBase* dataTree = foo();                                              // get a pointer to your data tree
  mitk::DataTreeFilter::Pointer treeFilter = mitk::DataTreeFilter::New(dataTree);    // create a filter
\endcode

  This will create a default filter that matches any mitk::Image in the tree. It will tell
  clients to display the "name" property of each image. 
  After calling the constructor, the filter is ready to be used.
  
  \subsubsection sectionMitkDataTreeFilterUsage12 Installing a different filter function

  If you want to access not only images, you have to assign a different filter function. A
  filter function must be able to return a bool for each DataTreeNode it is shown, i.e. 
  it must be a sub-type of mitk::DataTreeFilterFunction.

  The reason that this sub-classing is used instead of "real" C++ function pointers, is
  that we couldn't make a combination of templates and function pointers compile on both
  Windows and Linux, so we switched to this way of defining filter functions.
  
  The default filter IsImage is defined (almost) like this:

\code
  namespace mitk
  {
    class IsImage : public DataTreeFilterFunction
    {
      public:
        virtual ~IsImage() {}

        virtual bool NodeMatches(DataTreeNode* node) const
        {
          return (   node                                               // not a NULL pointer
                  && node->GetData()                                    // data in this node is not NULL either
                  && dynamic_cast<mitk::Image*>( node->GetData() ) );   // AND data is an mitk::Image (of some kind)
        }

        virtual DataTreeFilterFunction* Clone() const
        {
          return new IsImage();
        }
    };

  }
\endcode

  Subclass your own filter just like shown above and install them via SetFilter().

  Some oftenly used filters are collected in mitkDataTreeFilterFunctions.h. Have a look at
  them and add your own if they may be useful for others.
  
\code
  treeFilter->SetFilter( mitk::IsBaseDataTypeWithProperty<mitk::Image>("segmentation") );
\endcode
 
  \subsubsection sectionMitkDataTreeFilterUsage13 Defining the filter's behaviour

  You may want to define what kind of selections are possible and how the tree's hierarchy
  is handled. 
  
\code
  treeFilter->SetSelectionMode(mitk::DataTreeFilter::MULTI_SELECT);           // default behaviour
  treeFilter->SetSelectionMode(mitk::DataTreeFilter::SINGLE_SELECT);

  treeFilter->SetHierarchyHandling(mitk::DataTreeFilter::PRESERVE_HIERARCHY); // default behaviour
  treeFilter->SetHierarchyHandling(mitk::DataTreeFilter::FLATTEN_HIERARCHY);
\endcode
  
  The behaviour of the different modes will not surprise you:
  <ul>
    <li> MULTI_SELECT: any number of items may be selected
    <li> SINGLE_SELECT: only zero or one items may be selected. This is guarantied by the
          DataTreeFilter. If you call SetSelected() on an item, while another item is
          still selected, the Filter will simply deselect the other item, before your
          request is executed.
    <li> PRESERVE_HIERARCHY: items will have the same hierarchy as the data tree. If some
          nodes in the hierarchy do not match the filter function, children of those nodes
          will move up the hierarchy and become children of the first node that matches
          the filter (see diagram below).
    <li> FLATTEN_HIERARCHY: hierarchy will be destroyed. The order of items will be that
          of a preorder traversal.
  </ul> 
  
  \image html doc_mitkDataTreeFilterHierarchyHandling.png How hierarchy is handled when not all nodes match a filter: nodes move up the tree structure until there is a possible parent.
  \image latex doc_mitkDataTreeFilterHierarchyHandling.eps How hierarchy is handled when not all nodes match a filter: nodes move up the tree structure until there is a possible parent.

  \subsubsection sectionMitkDataTreeFilterUsage14 Telling GUI classes what properties to display

  If you keep the defaults, clients like the QmitkDataTreeComboBox or the
  QmitkDataTreeListView will display the "name" property of each item (not editable by the user).
  You can change this by defining two lists. The first list contains the <i>visible
  properties</i>. Only properties that are mentioned in this list are displayed. The
  second list holds <i>editable properties</i>, which should be presented in a way that
  the user can change them. The editable properties have to be a subset of the visible
  properties! This is required but not enforced or checked in any way, it is <b>your
  responsibility</b> to check this! 

  Both lists are given as a PropertyList, which is a typedef for a STL container:

\code
  mitk::DataTreeFilter::PropertyList visibleProperties;
  visibleProperties.push_back("visible");
  visibleProperties.push_back("color");
  visibleProperties.push_back("name");
  treeFilter->SetVisibleProperties( visibleProperties );
  
  mitk::DataTreeFilter::PropertyList editableProperties;
  editableProperties.push_back("visible");
  editableProperties.push_back("color");
  treeFilter->SetEditableProperties( editableProperties );
\endcode
  
  
  \subsubsection sectionMitkDataTreeFilterUsage15 Destroying a data tree filter

  As you keep a reference to the filter in a Pointer, which is a itk::SmartPointer, you
  destroy a DataTreeFilter just like most ITK things: assign NULL to the pointer.
  
\code
  treeFilter = NULL;
\endcode
  
  \subsection sectionMitkDataTreeFilterUsage2 Accessing the information behind a DataTreeFilter

  This section's purpose is to clarify how GUI elements can access the information, which
  is contained in a DataTreeFilter.
  
  \subsubsection sectionMitkDataTreeFilterUsage21 Iterating over all items 

  To visit all items, you need recursion unless you can be sure to get a flattened
  hierarchy. The following code example will only illustrate this recursive case, as the
  more simple case of a flat list can be learnt from that one, too.

\code
  void QmitkSomeGUIClass::GenerateItems()
  {
    // m_DataTreeFilter is our mitk::DataTreeFilter::Pointer that is already initialized 
  
    ProcessAllItems( m_DataTreeFilter->GetItems(), 0 );
  }

  void QmitkSomeGUIClass::ProcessAllItems(const mitk::DataTreeFilter::ItemList* items,
                                          int level)
  {
    // one iterator to the beginning of the list
    mitk::DataTreeFilter::ConstItemIterator itemiter( items->Begin() ); 
    // another one to the end of the list
    mitk::DataTreeFilter::ConstItemIterator itemiterend( items->End() ); 
  
    // visit each item until the end is reached
    while ( itemiter != itemiterend )
    {
  
      // work all visible properties
      for( mitk::DataTreeFilter::PropertyList::const_iterator nameiter = visibleProps.begin();
          nameiter != visibleProps.end();
          ++nameiter )
      {
        // get the "name" property and cast it to a string
        std::string displayedText( static_cast<const std::string> itemiter->GetProperty(*nameiter) );
  
        // do something useful with displayedText
      }
  
      // visit children if necessary
      if ( itemiter->HasChildren() )
          ProcessAllItems( itemiter->GetChildren(), visibleProps, level+1 );
    
      // visit next item
      ++itemiter; 
    }
  }
\endcode
  
  \subsubsection sectionMitkDataTreeFilterUsage22 Notifications / keeping in sync

  If you need to get informed about changes in the structure of the items, or about the
  selection state of individual items, you have to use the ITK Observer mechanism. We
  decided for this mechanism (and against Qt signals) to keep the tree filter toolkit
  independent.

  Somewhere very early, you will want to create observers:
  
\code
  QmitkSomeGUIClass::ConnectTreeFilterEvents()
  {
    // the author of this example is lazy and connects only to a single event
    // (TreeFilterUpdateAllEvent)
    itk::ReceptorMemberCommand<QmitkSomeGUIClass>::Pointer command = itk::ReceptorMemberCommand<QmitkSomeGUIClass>::New();
    command->SetCallbackFunction(this, &QmitkSomeGUIClass::updateAllHandler);
  
    // m_UpdateAllConnection is an "unsigned long"
    m_UpdateAllConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterUpdateAllEvent(), command);
  }
\endcode

  For the above code block to work, you have to define a member function, which handles
  all update-all events.
\code
  void QmitkSomeGUIClass::updateAllHandler( const itk::EventObject& )
  {
    GenerateItems();
  }
\endcode

  Later, probably in the destructor of your GUI element, you want to remove the observers:

\code
  QmitkSomeGUIClass::DisconnectTreeFilterEvents()
  {
    m_DataTreeFilter->RemoveObserver( m_UpdateAllConnection );
  } 
\endcode

  In a realistic scenario, you want to create handlers for all available events. For
  examples, please refer to the implementation of QmitkDataTreeComboBox or
  QmitkDataTreeListView. Here is only a description of when each event is triggered:

  <ul>
    <li> TreeFilterRemoveItemEvent: 
         a single item (but not its children) is removed
    <li> TreeFilterRemoveChildrenEvent:
         all children (and grandchildren, ...) of an item are removed
    <li> TreeFilterRemoveAllEvent: 
         all items of the tree filter are removed
    <li> TreeFilterSelectionChangedEvent: 
         the selection status of a single item has changed
    <li> TreeFilterItemAddedEvent: 
         an item was added
    <li> TreeFilterUpdateAllEvent:
         the filter underwent a greater reorganization, so you should consider all your
         information invalid (and update it)
  </ul>

  The information about where the change has occurred (in case of the add-event) or will
  occur shortly (remove-events) is contained in the itk::EventObject. You can access it
  like this:

\code
  void QmitkSomeGUIClass::removeItemHandler( const itk::EventObject& e )
  {
    const mitk::TreeFilterRemoveItemEvent& event( static_cast<const mitk::TreeFilterRemoveItemEvent&>(e) );
   
    const mitk::DataTreeFilter::Item* removedItem = event.GetChangedItem();

    // removedItem will be removed shortly, so do something clever now
  }
\endcode

\section sectionMitkDataTreeFilterUndocumented Untold stories 

  - What is the role of BasePropertyAccessor?
  - What happens if you provide empty lists of visible and editable properties?
  - Why all this const/non-const fuss?
  - Why did nobody remove EXTENDED_SELECT? It is not implemented! It's only an idea.

*/

namespace mitk
{

class DataTreeFilterFunction;

//------ DataTreeFilter ------------------------------------------------------------------

class MITK_CORE_EXPORT DataTreeFilter : public itk::Object
{
public:

  class Item; // forward declare

  typedef mitk::DataTreeFilter Self;

  typedef itk::SmartPointer<Self> Pointer;

  /// smart pointer to an item
  typedef itk::SmartPointer<Item> ItemPointer;

  /// An ordered group of Items 
  typedef itk::SmartPointerVectorContainer<unsigned int, Item> ItemList; 

  /// map tree nodes to their corresponding items
  typedef std::map<mitk::DataTreeNode*, Item*> TreeNodeItemMap; 

  /// A set of Items
  typedef std::set<Item*> ItemSet; 

  /// A set of DataTreeNodes
  typedef std::set<const DataTreeNode*> DataTreeNodeSet; 

  /// Iterator over Items
  typedef ItemList::Iterator      ItemIterator;

  /// Iterator over const Items
  typedef ItemList::ConstIterator ConstItemIterator;

  /// An ordered group of property keys (ordered by user)
  typedef std::vector<std::string> PropertyList;

  /// How the model should treat the data tree hierarchy
  typedef enum { PRESERVE_HIERARCHY = 0, FLATTEN_HIERARCHY = 1 } HierarchyHandling;

  /// What kind of selections should be possible
  typedef enum { SINGLE_SELECT = 0, MULTI_SELECT = 1, EXTENDED_SELECT = 2 } SelectionMode;

  /// Exception, that is thrown, when an not-editable property should be accessed as a
  /// non-const object
  class MITK_CORE_EXPORT NoPermissionException {};

  /// Objects of this class are returned by Item[string]
  /// This class provides access to a BaseProperty, where the permission to write
  /// depends on the constructor's bool parameter
  class MITK_CORE_EXPORT BasePropertyAccessor
  {
  public:
    /// bool parameter: whether property is editable/writeable
    BasePropertyAccessor(mitk::BaseProperty*, bool);

    /// Query, whether this item may be changed
    bool IsEditable() const;

    /// cast to a const BaseProperty
    operator const mitk::BaseProperty*();

    /// Cast to a writeable BaseProperty
    operator mitk::BaseProperty*();

    /// cast to a const string
    operator const std::string();

  private:
    /// Whether users of this class may write to the property
    bool m_Editable; 
    mitk::BaseProperty* m_Property;
  };

  /// One item of the filter result list or tree
  class MITK_CORE_EXPORT Item : public itk::LightObject
  {
    friend class mitk::DataTreeFilter;

  public:

    typedef itk::SmartPointer<Item> Pointer;

    /// \brief Items have 1:1 relations to DataTreeNodes and DataTreeFilters, a
    ///        position within all (sub)items and a parent.
    ///
    /// Requires a mitk::DataTreeBase, otherwise it has no meaning.
    /// Needs the DataTreeFilter it belongs to (for checking the editability (?) of
    /// properties).  Third parameter is the position within all siblings.
    /// Fourth parameter is the parent item.
    static Pointer New(mitk::DataTreeNode*, DataTreeFilter*, int, const Item*);

    const PropertyList GetVisibleProperties() const;

    /// Access to properties via their key
    BasePropertyAccessor GetProperty(const std::string&) const;

    /// Access possible child nodes
    // const because I don't want views to change the underlying list
    const ItemList* GetChildren() const;

    bool HasChildren() const;

    /// Position within the childs of its parent (first item has 0)
    int GetIndex() const;

    /// True, when item is an orphan
    /// (has no parent)
    bool IsRoot() const;

    /// True, when item is selected
    bool IsSelected() const;
    void SetSelected(bool);

    /// Returns the parent of this item (orphans throws exceptions)
    const Item* GetParent() const;

    /// Returns the associated DataTreeNode
    const mitk::DataTreeNode* GetNode() const;  

  protected:

    /// Intentionally hidden
    Item(); 
    ~Item(); 
    Item(mitk::DataTreeNode*, DataTreeFilter*, int, const Item*);

    /// Position within the childs of its parent (first item has 0)
    int m_Index;

    /// The parent of this item 
    const Item* const m_Parent;

    /// Child nodes of this node
    ItemList::Pointer m_Children;

    /// Our DataTreeFilter
    DataTreeFilter* m_TreeFilter;

    /// Our DataTreeNode
    DataTreeNode* m_Node;

    /// Whether this item is selected or not
    bool m_Selected;
  };

  /// No meaning without a mitk::DataTreeBase
  static Pointer New(mitk::DataTreeBase*);

  /// Alternative New() with iterator instead of DataTree
  static Pointer New(mitk::DataTreeIteratorBase*);

  const DataTreeBase* GetDataTree() const;

  /// Labels (for Header) of visible properties
  void SetPropertiesLabels(const PropertyList);
  const PropertyList& GetPropertiesLabels() const;

  /// Keys of visible properties.
  /// Special meaning of empty list: give all properties
  void SetVisibleProperties(const PropertyList);
  const PropertyList& GetVisibleProperties() const;

  /// Keys of editable properties. Subset of the visible properties!
  void SetEditableProperties(const PropertyList);
  const PropertyList& GetEditableProperties() const;

  /// The mitk::BaseRenderer, for which node properties are given
  void SetRenderer(const mitk::BaseRenderer*);
  const mitk::BaseRenderer* GetRenderer() const;

  /// Install the filter function
  void SetFilter(const DataTreeFilterFunction*);
  void SetFilter(const DataTreeFilterFunction&);
  const DataTreeFilterFunction* GetFilter() const;

  /// Sets a set of objects that should be displayed instead of the filtered datatree.
  /// This is used in conjunction with mitk::DataStorage.
  /// This method will overwrite any DataTreeFilterFunction that was set before it was called.
  void SetDataStorageResultset(const mitk::DataStorage::SetOfObjects* rs);

  /// Set the selection mode (single/multi)
  void SetSelectionMode(const SelectionMode);
  SelectionMode GetSelectionMode() const;

  /// Set whether the tree hierarchy will be preserved
  void SetHierarchyHandling(const HierarchyHandling);
  HierarchyHandling GetHierarchyHandling() const;

  /// Access the top level items that passed the filter
  const ItemList* GetItems() const;

  /// Access the top level items that were selected
  const ItemSet* GetSelectedItems() const;
  const Item* GetSelectedItem() const;
  const mitk::DataTreeIteratorClone GetIteratorToSelectedItem() const;

  const Item* FindItem(const DataTreeNode* node) const;

  /// Views can call this to select items
  void SelectItem(const Item*, bool selected = true);
  
  /// This will unselect all selected items
  void UnselectAll();

  /// This will delete all selected items from the data tree
  void DeleteSelectedItems();
  
  /// This will delete all selected items and their subitems data tree
  void DeleteSelectedItemsAndSubItems();

  /// change notifications from the data tree
  void TreeChange(const itk::EventObject &);

  /// Call with true to enable updates whenever the data tree changes. Default is false,
  /// which requires you to call Update() from time to time
  void SetAutoUpdate(bool);

  /// Force regeneration of the whole item list
  void Update();

  bool GetSelectMostRecentItemMode();
  void SetSelectMostRecentItemMode(bool automaticallySelectMostRecent);

protected:

  // intentionally hidden (itk::Object / SmartPointer)
  DataTreeFilter(mitk::DataTreeBase*);
  ~DataTreeFilter();

private:

  void ConnectTreeEvents();
  void DisconnectTreeEvents();

  const Item* FindItem(const DataTreeNode* node, ItemList* itemList) const;

  void AddMatchingChildren(mitk::DataTreeIteratorBase*, ItemList*, Item*, bool = true);

  void StoreCurrentSelectionState();
  
  // build the model from the data tree
  void GenerateModelFromTree();

  /// All items that passed the filter
  ItemList::Pointer m_Items;

  /// map tree nodes to their corresponding items
  TreeNodeItemMap m_Item;

  /// All items that were selected
  ItemSet m_SelectedItems;

  /// Pointer to a filter function
  DataTreeFilterFunction* m_Filter;

  /// The data tree filtered by this class
  mitk::DataTreeBase::Pointer m_DataTree;

  /// should we preserve the hierachy of the tree?
  HierarchyHandling m_HierarchyHandling;

  /// modes: single select, multi select, extended select
  SelectionMode m_SelectionMode;

  PropertyList m_PropertyLabels;
  PropertyList m_VisibleProperties;
  PropertyList m_EditableProperties;

  const mitk::BaseRenderer* m_Renderer;

  unsigned long  m_TreeChangeConnection;

  // remember the most recently selected item
  //ItemPointer m_LastSelectedItem;
  Item* m_LastSelectedItem;

  const DataTreeNode* m_LastSelectedNode; /// for reconstruction of selection after regeneration of items
  DataTreeNodeSet m_LastSelectedNodes; /// for reconstruction of selection after regeneration of items

  bool m_AutoUpdate;

  bool m_SelectMostRecentItemMode;

#ifndef NDEBUG
public:
  void SetDebugOn() { m_DEBUG = true; }
  void PrintCurrentState() { PrintStateForDebug( std::cout ); }
private:
  void PrintStateForDebug(std::ostream& out);
  bool m_DEBUG;
#endif
};

} // namespace mitk

#endif
// vi: textwidth=90
