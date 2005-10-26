#include <mitkDataTree.h> 
#include <itkObject.h>

/*

   TODO / Ideen:

   - die View-Klassen bekommen Nachrichten ala "ItemChanged( item& )" - für ein einzelnes
     Item mit allen Properties,  "ItemChanged( item&, string key )" - für einzelne
     Properties eines Items, evtl auch "ModelChanged()" - wenn die View den kompletten
     Baum neu rendern soll.

     Über die member GetIndex() und IsRoot() GetParent() kann die View feststellen, welches
     ViewItem (Qt) gemeint ist (oder die View hält sich da irgendeine Tabelle für die
     Zuordnung). GetIndex() liefert die Position unterhalb vom Parent, und wenn
     IsRoot() true liefert, dann ist man schon auf höchster Ebene. 
  
  - beim ersten Update traversieren die Views die ganze Struktur und erzeugen sich
    ViewItems, die 1:1 Items entsprechen und als Observer an den Items hängen

  Die ersten beiden sachen passen nicht zusammen. nochmal drüber nachdenken!

*/


namespace mitk
{

  /// Inherits itk::Object for Subject/Observer functionality
  class DataTreeFilter : public itk::Object
  {
    public:

      class Item; // forward declare
      class RootItem; // forward declare
     
      typedef itk::SmartPointer<DataTreeFilter> Pointer;
      
      /// Defines a function for filtering tree nodes.
      typedef bool(*FilterFunctionPointer)(mitk::DataTreeNode*);
      
      /// An ordered group of RootItems
      typedef std::vector<RootItem> RootItemList;
      
      /// An ordered group of Items
      typedef std::vector<Item> ItemList;

      /// Iterator over (?) Items
      typedef std::vector<Item>::iterator ItemIterator;
      
      /// An ordered group of property keys
      typedef std::vector<std::string> PropertyList;

      /// Exception, that is thrown, when an not-editable property should be accessed as a
      /// non-const object
      class NoPermissionException {};
     
      /// Exception, that is thrown, when one accesses the parent of an item, that has no 
      /// parent (because GetParent() can't return a NULL reference.
      /// Can be avoided by checking IsRoot() prior to GetParent()
      class HasNoParent {};
    
      /// Objects of this class are returned by Item[string]
      /// This class provides access to a BaseProperty, where the permission to write
      /// depends on the constructor's bool parameter
      class BasePropertyAccessor
      {
        public:
          /// bool parameter: whether property is editable/writeable
          BasePropertyAccessor(mitk::BaseProperty&, bool);
       
          /// Query, whether this item may be changed
          bool IsEditable() const;
          
          /// cast to a const BaseProperty
          operator const mitk::BaseProperty&();
          
          /// Cast to a writeable BaseProperty
          operator mitk::BaseProperty&();
        
        private:
          /// Wheter users of this class may write to the property
          bool m_Editable; 
          mitk::BaseProperty& m_Property;
      };
      
      /// One item of the filter result list or tree
      class Item
      {
        public:

          /// \brief Items have 1:1 relations to DataTreeNodes and DataTreeFilters, a
          ///        position within all (sub)items and a parent.
          ///
          /// Requires a mitk::DataTree, otherwise it has no meaning.
          /// Needs the DataTreeFilter it belongs to (for checking the editability (?) of
          /// properties).  Third parameter is the position within all siblings.
          /// Fourth parameter is the parent item.
          Item(mitk::DataTreeNode&, DataTreeFilter&, int, const Item&);

          /// Access to properties via their key
          BasePropertyAccessor& operator[](const std::string&) const;

          /// Access possible child nodes
          const ItemList& GetChildren() const;

          /// Position within the childs of its parent (first item has 0)
          int GetIndex() const;

          /// True, when item is an orphan
          /// (has no parent)
          bool IsRoot() const;

          /// Returns the parent of this item (orphans throws exceptions)
          const Item& GetParent() const;
        protected:

          /// Intentionally hidden
          Item(); 
          
          /// For derived classes
          Item(mitk::DataTreeNode&, DataTreeFilter&, int);

          /// Position within the childs of its parent (first item has 0)
          int m_Index;
          
          /// The parent of this item 
          const Item* const m_Parent;
          
          /// Child nodes of this node
          ItemList m_Children;

          /// Our DataTreeFilter
          DataTreeFilter& m_TreeFilter;

          /// Our DataTreeNode
          DataTreeNode& m_Node;
      };

      class RootItem : public Item
      {
        public:
          
          /// \brief RootItems have 1:1 relations to DataTreeNodes and DataTreeFilters, a
          ///        position within all (sub)items but no parent.
          ///
          /// Requires a mitk::DataTree, otherwise it has no meaning.
          /// Needs the DataTreeFilter it belongs to (for checking the editability (?) of
          /// properties). Third parameter is the position within all siblings.
          RootItem(mitk::DataTreeNode&, DataTreeFilter&, int);

        private:
          // just hidden
          RootItem(mitk::DataTreeNode&, DataTreeFilter&, int, const Item& parent);
      };
     
      /// No meaning without a mitk::DataTree 
      static Pointer New(mitk::DataTree&);


      /// Keys of visible properties
      void SetVisibleProperties(PropertyList);
      const PropertyList& GetVisibleProperties() const;
      
      /// Labels (for Header) of visible properties
      void SetPropertiesLabels(PropertyList);
      const PropertyList& GetPropertiesLabels() const;

      /// Keys of editable properties. Subset of the visible properties!
      void SetEditableProperties(PropertyList);
      const PropertyList& GetEditableProperties() const;

      /// The mitk::BaseRenderer, for which node properties are given
      void SetRenderer(const mitk::BaseRenderer*);

      /// Install the filter function
      void SetFilter(FilterFunctionPointer);

      /// Access the top level items that passed the filter
      const RootItemList& GetItems() const;
      
    private:
    
      // intentionally hidden (itk::Object / SmartPointer)
      DataTreeFilter(mitk::DataTree&);
      ~DataTreeFilter();

      /// All items that passed the filter
      RootItemList m_Items;

      /// Pointer to a filter function
      FilterFunctionPointer m_Filter;

      /// The data tree filtered by this class
      mitk::DataTree& m_DataTree;
  };
  

} // namespace mitk


// vi: textwidth=90
