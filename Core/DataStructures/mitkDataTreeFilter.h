#ifndef MITK_DATATREEFILTER_H_INCLUDED
#define MITK_DATATREEFILTER_H_INCLUDED

#include <mitkDataTree.h> 
#include <itkSmartPointerVectorContainer.h>
#include "itkMacro.h"

/*

   TODO / Ideen:

   - Die views bekommen vom Filter nur Infos über hinzufügen/löschen von items.

   - für änderungen der properties müssen sich die views selbst bei denen anmelden

     Über die member GetIndex() und IsRoot() GetParent() kann die View feststellen, welches
     ViewItem (Qt) gemeint ist (oder die View hält sich da irgendeine Tabelle für die
     Zuordnung). GetIndex() liefert die Position unterhalb vom Parent, und wenn
     IsRoot() true liefert, dann ist man schon auf höchster Ebene. 

   - Die Auswahl (welche Items sind selektiert) gehört in das Modell. Damit kann man in
     mehreren Views die Auswahl konsistent halten (Combo- und Listbox).

*/

namespace mitk
{

//------ DataTreeFilter ------------------------------------------------------------------

  /// Inherits itk::Object for Subject/Observer functionality
  class DataTreeFilter : public itk::Object
  {
    public:

      class Item; // forward declare

      typedef mitk::DataTreeFilter Self;
     
      typedef itk::SmartPointer<Self> Pointer;
      
      /// Defines a function for filtering tree nodes.
      typedef bool(*FilterFunctionPointer)(mitk::DataTreeNode*);
     
      /// smart pointer to an item
      typedef itk::SmartPointer<Item> ItemPointer;
      
      /// An ordered group of Items 
      typedef itk::SmartPointerVectorContainer<unsigned int, Item> ItemList; 
     
      /// map tree nodes to their corresponding items
      typedef std::map<mitk::DataTreeNode*, Item*> TreeNodeItemMap; 
     
      /// A set of Items
      typedef std::set<Item*> ItemSet; 

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
      class NoPermissionException {};
     
      /// Objects of this class are returned by Item[string]
      /// This class provides access to a BaseProperty, where the permission to write
      /// depends on the constructor's bool parameter
      class BasePropertyAccessor
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
      class Item : public itk::LightObject
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

          /// 
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
      void SetFilter(FilterFunctionPointer);
      const FilterFunctionPointer GetFilter() const;

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

      /// Views can call this to select items
      void SelectItem(const Item*, bool selected = true);
      
      /// change notifications from the data tree
      void TreeNodeChange(const itk::EventObject &);
      void TreeAdd(const itk::EventObject &);
      void TreePrune(const itk::EventObject &);
      void TreeRemove(const itk::EventObject &);
      
    protected:
   
      // intentionally hidden (itk::Object / SmartPointer)
      DataTreeFilter(mitk::DataTreeBase*);
      ~DataTreeFilter();
    
    private:

      void AddMatchingChildren(mitk::DataTreeIteratorBase*, ItemList*, Item*, bool = true);
      
      // build the model from the data tree
      void GenerateModelFromTree();

      /// All items that passed the filter
      ItemList::Pointer m_Items;
      
      /// map tree nodes to their corresponding items
      TreeNodeItemMap m_Item;

      /// All items that were selected
      ItemSet m_SelectedItems;

      /// Pointer to a filter function
      FilterFunctionPointer m_Filter;

      /// The data tree filtered by this class
      mitk::DataTreeBase* m_DataTree;

      /// should we preserve the hierachy of the tree?
      HierarchyHandling m_HierarchyHandling;

      /// modes: single select, multi select, extended select
      SelectionMode m_SelectionMode;

      PropertyList m_PropertyLabels;
      PropertyList m_VisibleProperties;
      PropertyList m_EditableProperties;

      const mitk::BaseRenderer* m_Renderer;

      unsigned long  m_TreeNodeChangeConnection;
      unsigned long  m_TreeAddConnection;
      unsigned long  m_TreePruneConnection;
      unsigned long  m_TreeRemoveConnection;
      
      // remember the most recently selected item
      ItemPointer m_LastSelectedItem;
  };
  
  bool IsDataTreeNode(mitk::DataTreeNode*);
  bool IsGoodDataTreeNode(mitk::DataTreeNode*);

} // namespace mitk

#endif
// vi: textwidth=90

