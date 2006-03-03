#include <itkCommand.h>
#include <mitkDataTreeFilter.h>
#include <mitkDataTreeFilterEvents.h>
#include <mitkDataTree.h>
#include <mitkDataTreeHelper.h>
#include <mitkPropertyManager.h>

namespace mitk
{

//------ Some common filter functions ----------------------------------------------------

/// default filter, lets everything except NULL pointers pass
bool IsDataTreeNode(DataTreeNode* node)
{
  return ( node!= 0 );
}
  
bool IsGoodDataTreeNode(DataTreeNode* node)
{
  return ( node!= 0 && node->GetData() );
}

bool IsImage(DataTreeNode* node)
{
  return ( node!= 0 && node->GetData() && dynamic_cast<Image*>( node->GetData() ) );
}

//------ BasePropertyAccessor ------------------------------------------------------------
  
DataTreeFilter::BasePropertyAccessor::BasePropertyAccessor(BaseProperty* property, bool editable)
: m_Editable(editable),
  m_Property(property)
{
  // nothing to do
}

bool DataTreeFilter::BasePropertyAccessor::IsEditable() const
{
  return m_Editable;
}

DataTreeFilter::BasePropertyAccessor::operator const BaseProperty*()
{
  // const access always permitted
  return m_Property;
}

DataTreeFilter::BasePropertyAccessor::operator BaseProperty*()
{
  // non-const access only permitted when editable
  if (!m_Editable) throw NoPermissionException();
  return m_Property;
}

DataTreeFilter::BasePropertyAccessor::operator const std::string()
{
  if (!m_Property) 
    return std::string("[no value]");
  else
    return m_Property->GetValueAsString();
}

//------ Item ----------------------------------------------------------------------------
DataTreeFilter::Item::Pointer DataTreeFilter::Item::New(DataTreeNode* node, DataTreeFilter* treefilter, 
                                                        int index, const Item* parent)
{
  // from itkNewMacro()
  Pointer smartPtr;
  Item* rawPtr = new Item(node, treefilter, index, parent);
  smartPtr = rawPtr;
  rawPtr->UnRegister();
  treefilter->m_Item[node] = rawPtr;
  return smartPtr;
}

DataTreeFilter::Item::Item(DataTreeNode* node, DataTreeFilter* treefilter, 
                           int index, const Item* parent)
: m_Index(index),
  m_Parent(parent),
  m_Children(NULL),
  m_TreeFilter(treefilter),
  m_Node(node),
  m_Selected(false)
{
  // ensure we always have a vaild node pointer
  if (!node) throw std::invalid_argument("NULL pointer makes no sense in DataTreeFilter::Item::Item()");
    
  m_Children = DataTreeFilter::ItemList::New();
}

DataTreeFilter::Item::~Item()
{
  // remove selection
  m_TreeFilter->m_SelectedItems.erase(this);
  // remove map entry
  m_TreeFilter->m_Item.erase(m_Node);
}

const DataTreeFilter::PropertyList DataTreeFilter::Item::GetVisibleProperties() const
{
  if ( m_TreeFilter->m_VisibleProperties.empty() )
  {
    // special case of empty list: return all the properties of a data tree node
    const mitk::PropertyList::PropertyMap* prop_map = m_Node->GetPropertyList(m_TreeFilter->GetRenderer())-> GetMap();

    mitk::PropertyList::PropertyMap::const_iterator iter;
    PropertyList result_list;
    for ( iter = prop_map->begin(); iter != prop_map->end(); ++iter )
    {
      if ( iter->second.second )                  // if this property IS enabled
        result_list.push_back( iter->first );     // add it to the results list
    }
    
    return result_list;
  }
  else
  {
    return m_TreeFilter->m_VisibleProperties;
  }
}

DataTreeFilter::BasePropertyAccessor DataTreeFilter::Item::GetProperty(const std::string& key) const
{
  BaseProperty* prop = m_Node->GetProperty(key.c_str(), m_TreeFilter->m_Renderer);
 
  // No access to property, if: not all properties are accessible (visible_list is not empty)
  //                            AND the requested property cannot be found in visible_list
  if (    !m_TreeFilter->m_VisibleProperties.empty()
      &&  std::find( m_TreeFilter->m_VisibleProperties.begin(), m_TreeFilter->m_VisibleProperties.end(), key ) 
             == m_TreeFilter->m_VisibleProperties.end() )
  {
    // key not marked visible
    return BasePropertyAccessor(0, false);
  }

  if ( !prop ) // property not found in list (or "disabled")
  {
    BaseProperty::Pointer defaultProp = // create a default property
      PropertyManager::GetInstance()->CreateDefaultProperty(key.c_str());

    if ( defaultProp.IsNotNull() )
    {
      m_Node->SetProperty( key.c_str(), defaultProp );
      prop = defaultProp;
    }
  }

  // visible. determine whether property may be edited
  // Editable, if: all properties are accessible (visible_list is empty)
  //               or only certain properties are accessible AND the requested property is
  //               in the editable_list
  bool editable = 
       m_TreeFilter->m_VisibleProperties.empty()
    || std::find(m_TreeFilter->m_EditableProperties.begin(), m_TreeFilter->m_EditableProperties.end(), key)
         != m_TreeFilter->m_EditableProperties.end();

  return BasePropertyAccessor(prop, editable); 
}

const DataTreeFilter::ItemList* DataTreeFilter::Item::GetChildren() const
{
  return m_Children;
}

bool DataTreeFilter::Item::HasChildren() const
{
  return m_Children->Size() > 0;
}

int DataTreeFilter::Item::GetIndex() const
{
  return m_Index;
}

bool DataTreeFilter::Item::IsRoot() const
{
  return m_Parent == NULL;
}

bool DataTreeFilter::Item::IsSelected() const
{
  return m_Selected;
}

void DataTreeFilter::Item::SetSelected(bool selected)
{
  if ( selected != m_Selected )
  {
    m_Selected = selected;

    if ( selected )
    {
      if ( m_TreeFilter->m_SelectionMode == DataTreeFilter::SINGLE_SELECT &&
          m_TreeFilter->m_LastSelectedItem && m_TreeFilter->m_LastSelectedItem != this)
// TODO this compares smartpointers with "weak" pointers. Rewrite
//            if ( std::find( m_TreeFilter->m_Items->begin(), m_TreeFilter->m_Items->end(), m_TreeFilter->m_LastSelectedItem ) != m_TreeFilter->m_Items->end() )
          m_TreeFilter->m_LastSelectedItem->SetSelected(false);

      m_TreeFilter->m_LastSelectedItem = this;
      m_TreeFilter->m_SelectedItems.insert(this);
    }
    else
    {
      m_TreeFilter->m_SelectedItems.erase(this);
      if (m_TreeFilter->m_LastSelectedItem == this)
        m_TreeFilter->m_LastSelectedItem = NULL;
    }
    m_TreeFilter->InvokeEvent( TreeFilterSelectionChangedEvent(this, selected) );
  }
}

const DataTreeFilter::Item* DataTreeFilter::Item::GetParent() const
{
  return m_Parent;
}

const DataTreeNode* DataTreeFilter::Item::GetNode() const
{
  return m_Node;
}

//------ DataTreeFilter ------------------------------------------------------------------

// smart pointer constructor
DataTreeFilter::Pointer DataTreeFilter::New(DataTreeBase* datatree)
{
  // from itkNewMacro()
  Pointer smartPtr;
  DataTreeFilter* rawPtr = new DataTreeFilter(datatree);
  smartPtr = rawPtr;
  rawPtr->UnRegister();
  return smartPtr;
}

// real constructor (protected)
DataTreeFilter::DataTreeFilter(DataTreeBase* datatree)
: m_Filter(NULL),
  m_DataTree(datatree),
  m_HierarchyHandling(DataTreeFilter::PRESERVE_HIERARCHY),
  m_SelectionMode(DataTreeFilter::MULTI_SELECT),
  m_Renderer(NULL),
  m_TreeNodeChangeConnection(0),
  m_TreeAddConnection(0),
  m_TreePruneConnection(0),
  m_TreeRemoveConnection(0),
  m_LastSelectedItem(NULL)
{
  //SetFilter( &IsDataTreeNode );
  SetFilter( &IsImage );
  m_VisibleProperties.push_back("name");
  
  m_Items = ItemList::New(); // create an empty list

  // connect tree notifications to member functions
  {
  itk::ReceptorMemberCommand<DataTreeFilter>::Pointer command = itk::ReceptorMemberCommand<DataTreeFilter>::New();
  command->SetCallbackFunction(this, &DataTreeFilter::TreeNodeChange);
  m_TreeNodeChangeConnection = m_DataTree->AddObserver(itk::TreeNodeChangeEvent<DataTreeBase>(), command);
  }

  {
  itk::ReceptorMemberCommand<DataTreeFilter>::Pointer command = itk::ReceptorMemberCommand<DataTreeFilter>::New();
  command->SetCallbackFunction(this, &DataTreeFilter::TreeAdd);
  m_TreeAddConnection = m_DataTree->AddObserver(itk::TreeAddEvent<DataTreeBase>(), command);
  }
  
  {
  itk::ReceptorMemberCommand<DataTreeFilter>::Pointer command = itk::ReceptorMemberCommand<DataTreeFilter>::New();
  command->SetCallbackFunction(this, &DataTreeFilter::TreeRemove);
  m_TreeRemoveConnection = m_DataTree->AddObserver(itk::TreeRemoveEvent<DataTreeBase>(), command);
  }

  {
  itk::ReceptorMemberCommand<DataTreeFilter>::Pointer command = itk::ReceptorMemberCommand<DataTreeFilter>::New();
  command->SetCallbackFunction(this, &DataTreeFilter::TreePrune);
  m_TreePruneConnection = m_DataTree->AddObserver(itk::TreePruneEvent<DataTreeBase>(), command);
  }

  
  GenerateModelFromTree();
}

DataTreeFilter::~DataTreeFilter()
{
  // remove this as observer from the data tree
  m_DataTree->RemoveObserver( m_TreeNodeChangeConnection );
  m_DataTree->RemoveObserver( m_TreeAddConnection );
  m_DataTree->RemoveObserver( m_TreeRemoveConnection );
  m_DataTree->RemoveObserver( m_TreePruneConnection );
  
  InvokeEvent( TreeFilterRemoveAllEvent() );

  m_Items->clear();
  m_Item.clear(); // should already BE clear by now
  m_SelectedItems.clear();
}

void DataTreeFilter::SetPropertiesLabels(const PropertyList labels)
{
  m_PropertyLabels = labels;
  GenerateModelFromTree();
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetPropertiesLabels() const
{
  return m_PropertyLabels;
}

void DataTreeFilter::SetVisibleProperties(const PropertyList keys)
{
  m_VisibleProperties = keys;
  GenerateModelFromTree();
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetVisibleProperties() const
{
  return m_VisibleProperties;
}
      
void DataTreeFilter::SetEditableProperties(const PropertyList keys)
{
  m_EditableProperties = keys;
  GenerateModelFromTree();
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetEditableProperties() const
{
  return m_EditableProperties;
}

void DataTreeFilter::SetRenderer(const BaseRenderer* renderer)
{
  m_Renderer = renderer;
  GenerateModelFromTree();
}
      
const BaseRenderer* DataTreeFilter::GetRenderer() const
{
  return m_Renderer;
}

void DataTreeFilter::SetFilter(FilterFunctionPointer filter)
{
  if (m_Filter == filter) return;
  
  if (filter)
    m_Filter = filter;
  else
    m_Filter = &IsImage;

  GenerateModelFromTree();
}
      
const DataTreeFilter::FilterFunctionPointer DataTreeFilter::GetFilter() const
{
  return m_Filter;
}
      
void DataTreeFilter::SetSelectionMode(const SelectionMode selectionMode)
{
  if (m_SelectionMode == selectionMode) return;
  
  m_SelectionMode = selectionMode;

  GenerateModelFromTree();
  // TODO update selection (if changed from multi to single)
  // InvokeEvent( TreeFilterSelectionChanged( item, selected ) );
}

DataTreeFilter::SelectionMode DataTreeFilter::GetSelectionMode() const
{
  return m_SelectionMode;
}
      
void DataTreeFilter::SetHierarchyHandling(const HierarchyHandling hierarchyHandling)
{
  if (m_HierarchyHandling == hierarchyHandling) return;
  
  m_HierarchyHandling = hierarchyHandling;
  
  GenerateModelFromTree();
}

DataTreeFilter::HierarchyHandling DataTreeFilter::GetHierarchyHandling() const
{
  return m_HierarchyHandling;
}

const DataTreeFilter::ItemList* DataTreeFilter::GetItems() const
{
  return m_Items;
}
      
const DataTreeFilter::ItemSet* DataTreeFilter::GetSelectedItems() const
{
  return &m_SelectedItems;
}

const DataTreeFilter::Item* DataTreeFilter::GetSelectedItem() const
{
  return m_LastSelectedItem;
}

const DataTreeIteratorClone DataTreeFilter::GetIteratorToSelectedItem() const
{
  if ( m_LastSelectedItem && m_DataTree )
  {
    return DataTreeHelper::FindIteratorToNode(m_DataTree, m_LastSelectedItem->GetNode());
  }
  
  return NULL;
}

const DataTreeFilter::Item* DataTreeFilter::FindItem(DataTreeNode* node, DataTreeFilter::ItemList* itemList)
{
  ItemList::iterator listIter;
  for ( listIter = itemList->begin(); listIter != itemList->end(); ++listIter )
  {
    if ( (*listIter)->GetNode() == node )
      return *listIter;
    else if ( (*listIter)->HasChildren() )
    {
      const DataTreeFilter::Item* item( FindItem( node, (*listIter)->m_Children ) );
      if (item) return item;
    }
  }

  return NULL;
}

const DataTreeFilter::Item* DataTreeFilter::FindItem(DataTreeNode* node)
{
  return FindItem(node, m_Items);
}
      
void DataTreeFilter::SelectItem(const Item* item, bool selected)
{
  // Ich const_caste mir die Welt, wie sie mir gefällt :-)
  // Aber die Items gehören ja dem Filter, deshalb ist das ok
  const_cast<Item*>(item) -> SetSelected(selected);
}

void DataTreeFilter::TreeNodeChange(const itk::EventObject& e)
{
  if ( typeid(e) != typeid(itk::TreeNodeChangeEvent<DataTreeBase>) ) return;
 
  // if event.GetChangePosition()->GetNode() == NULL, then something was removed (while in ~DataTree()) by setting it to NULL
  const itk::TreeChangeEvent<DataTreeBase>& event( static_cast<const itk::TreeChangeEvent<DataTreeBase>&>(e) );
  DataTreeIteratorBase* treePosition = const_cast<DataTreeIteratorBase*>(&(event.GetChangePosition()));

  if ( treePosition->Get().IsNull() ) return; // TODO this special case is used only in
                                              // ~DataTree(). If used anywhere else, this
                                              // line here has to be removed. But probably
                                              // nobody should set a node to NULL

  // nothing is known about real TreeNodeChange events so we play safe and regenerate all
  GenerateModelFromTree();
}

/// @todo Could TreeAdd be implemented more intelligent?
void DataTreeFilter::TreeAdd(const itk::EventObject& e)
{
  std::cout << (void*)this << " TreeAddEvent" << std::endl;
  /// Regenerate item tree only from the changed position on (when hierarchy is preserved,
  /// otherwise rebuild everything)
  const itk::TreeAddEvent<DataTreeBase>& event( static_cast<const itk::TreeAddEvent<DataTreeBase>&>(e) );
  DataTreeIteratorBase* treePosition = const_cast<DataTreeIteratorBase*>(&(event.GetChangePosition()));

  if ( !m_Filter(treePosition->Get()) ) return; // if filter does not match, then nothing has to be done

  /*
     find out, whether there is an item that will be this new node's parent
     if there is, regenerate the item tree from that parent on
     otherwise, regenerate all items (because then this item will be part of
       the top level items
  */
  
  ItemList* list(m_Items);
  Item* parent(0);

  if ( m_HierarchyHandling == DataTreeFilter::PRESERVE_HIERARCHY )
    while ( treePosition->GoToParent() )
    {
      if ( m_Filter(treePosition->Get()) )
      {                                         // this is the new parent
        parent = m_Item[treePosition->Get()];
          if (!parent) 
          {
            std::cerr << "l. " << __LINE__ << ": Treefilter " << (void*)this << " looked for m_Item[...] but couldn't find anything." << std::endl;
            return; // TODO is that possible? At least this happens sometimes, may be a bug
          }
        break; 
      }
    }

  if (parent)
  {
    // regenerate only in part
    list = parent->m_Children;
    InvokeEvent( TreeFilterRemoveChildrenEvent( parent ) );
    list->clear();
    AddMatchingChildren( treePosition, list, parent, true );
  }
  else
  {
    GenerateModelFromTree();
  }
  
}

void DataTreeFilter::TreePrune(const itk::EventObject& e)
{
  std::cout << (void*)this << " TreePruneEvent" << std::endl;
  // event has a iterator to the node that is about to be deleted
  const itk::TreeRemoveEvent<DataTreeBase>& event( static_cast<const itk::TreeRemoveEvent<DataTreeBase>&>(e) );
  DataTreeIteratorBase* treePosition = const_cast<DataTreeIteratorBase*>(&(event.GetChangePosition()));
  
  /*
    if hierarchy is preserved AND if the event's position matches the filter (i.e. an item exists)
        remove that one item from its parent's list
    else      // sub-nodes could be removed that match our filter, even if the removed
              // node does not match
      get preorder iterator from the event's position on
      each iterator position: check for filter matches
        for the first match, 
          get the corresponding item
          mark this item's parent as LIST
          mark the items position in LIST as BEGIN and as END
        for each following match
          increase the position of END
      in LIST: erase all items from BEGIN to END
  */

  Item* item(0);
  ItemList* list(0);
  ItemList::iterator listFirstIter;
  ItemList::iterator listLastIter;
  
  if (   m_HierarchyHandling == DataTreeFilter::PRESERVE_HIERARCHY 
      && m_Filter(treePosition->Get()) )
  {
    item = m_Item[ treePosition->Get() ];
          if (!item) 
          {
            std::cerr << "l. " << __LINE__ << ": Treefilter " << (void*)this << " looked for m_Item[...] but couldn't find anything." << std::endl;
            return; // TODO is that possible? At least this happens sometimes, may be a bug
          }

    if ( item->IsRoot() )
      list = m_Items;
    else
      list = item->m_Parent->m_Children;
    
    for ( listFirstIter = list->begin(); listFirstIter != list->end(); ++listFirstIter )
      if ( *listFirstIter == item )
      {
        listLastIter = listFirstIter;
        break;
      }
  }
  else // no hierachy preserved OR removed data tree item not matching the filter
  {
    DataTreePreOrderIterator treeIter( m_DataTree, treePosition->GetNode() );
    bool firstMatch(true);
    while ( !treeIter.IsAtEnd() )
    {
      if ( m_Filter(treeIter.Get()) )
      {
        if ( firstMatch )
        {
          item = m_Item[ treeIter.Get() ];
          if (!item) 
          {
            std::cerr << __FILE__ << " l. " << __LINE__ << ": Treefilter " << (void*)this << " looked for m_Item[...] but couldn't find anything." << std::endl;
            return; // TODO is that possible? At least this happens sometimes, may be a bug
          }
          
          if ( item->IsRoot() )
            list = m_Items;
          else
            list = item->m_Parent->m_Children;
  
          for ( listFirstIter = list->begin(); listFirstIter != list->end(); ++listFirstIter )
            if ( *listFirstIter == item )
            {
              listLastIter = listFirstIter;
              break;
            }

          firstMatch = false;
        }
        else // later matches
        {
          ++listLastIter;
        }
      }
      ++treeIter;    
    }
  }

  if (item)
  {
    // clean selected list
    ++listLastIter; // erase does delete until the item _before_ the second iterator

    InvokeEvent( TreeFilterRemoveChildrenEvent( item->m_Parent ) );  // first tell everything is deleted
    list->erase( listFirstIter, listLastIter );
  
    // renumber items of the remaining list
    int i(0);
    for ( listFirstIter = list->begin(); listFirstIter != list->end(); ++listFirstIter, ++i )
    {
      (*listFirstIter)->m_Index = i;
      InvokeEvent( TreeFilterItemAddedEvent( *listFirstIter ) );  // then add some items again
    }
  }
}

void DataTreeFilter::TreeRemove(const itk::EventObject& e)
{
  if ( typeid(e) != typeid(itk::TreeRemoveEvent<DataTreeBase>) ) return;
  
  // event has a iterator to the node that is about to be deleted
  const itk::TreeRemoveEvent<DataTreeBase>& event( static_cast<const itk::TreeRemoveEvent<DataTreeBase>&>(e) );
  DataTreeIteratorBase* treePosition = const_cast<DataTreeIteratorBase*>(&(event.GetChangePosition()));
  
  if ( m_Filter(treePosition->Get()) )
  {
    Item* item = m_Item[ treePosition->Get() ];
          if (!item) 
          {
            std::cerr << "l. " << __LINE__ << ": Treefilter " << (void*)this << " looked for m_Item[...] but couldn't find anything." << std::endl;
            return; // TODO is that possible? At least this happens sometimes, may be a bug
          }
    ItemList* list(0);
    
    if ( item->IsRoot() )
      list = m_Items;
    else
      list = item->m_Parent->m_Children;
    
    ItemList::iterator listIter;
    for ( listIter = list->begin(); listIter != list->end(); ++listIter )
      if ( *listIter == item ) break;

    InvokeEvent( TreeFilterRemoveChildrenEvent( item->m_Parent ) );
    
    if ( item->m_Children->size() > 0 )
    {
      list->insert(listIter, item->m_Children->begin(), item->m_Children->end());
    }
  
    // because I'm not sure if the STL guarantees something about the position of an
    // iterator after insert, I once again look for the item to remove

    for ( listIter = list->begin(); listIter != list->end(); ++listIter )
      if ( *listIter == item ) break;
    
    InvokeEvent( TreeFilterRemoveItemEvent(item) );
    list->erase(listIter);
  
    // renumber items of the remaining list
    int i(0);
    for ( listIter = list->begin(); listIter != list->end(); ++listIter, ++i )
    {
      (*listIter)->m_Index = i;
      InvokeEvent( TreeFilterItemAddedEvent( *listIter ) );
    }
  }
}

void DataTreeFilter::AddMatchingChildren(DataTreeIteratorBase* iter, ItemList* list,
    Item* parent, bool verbose )
{
  /* 
  for each child of iter
    check if filter matches
    if it matches:
      create an Item for this child
      iter->GoToChild(thisChild)
      call AddMatchingChildren(iter, newItem.m_ItemList)
      iter->GotoParent();
    if match fails:
      iter->GoToChild(thisChild)
      call AddMatchingChildren(iter, list)  // list from this call
      iter->GotoParent();
  
  Consideration of m_HierarchyHandling adds one branch
  */
  int numChildren( iter->CountChildren() );
  for ( int child = 0; child < numChildren; ++child )
  {
    iter->GoToChild(child);
    if ( m_Filter( iter->Get() ) )
    {
      unsigned int newIndex = list->Size();
      list->CreateElementAt( newIndex ) = Item::New( iter->Get(), this, newIndex, parent );
      if (verbose) InvokeEvent( TreeFilterItemAddedEvent(m_Items->ElementAt(newIndex)) );

      if ( m_HierarchyHandling == DataTreeFilter::PRESERVE_HIERARCHY )
      {
        AddMatchingChildren( iter, 
                            list->ElementAt(newIndex)->m_Children,
                            list->ElementAt(newIndex).GetPointer(), verbose );
      }
      else
      {
        AddMatchingChildren( iter, list, parent, verbose );
      }
    }
    else
    {
      AddMatchingChildren( iter, list, parent, verbose );
    }
    iter->GoToParent();
  }
}

void DataTreeFilter::GenerateModelFromTree()
{
  InvokeEvent( TreeFilterRemoveAllEvent() );

  m_Items = ItemList::New(); // clear list (nice thanks to smart pointers)
  DataTreeIteratorBase* treeIter =  // get an iterator to the data tree
    new DataTreePreOrderIterator(m_DataTree);
  
  /*
  if root matches
    create an Item for root
    call AddMatchingChildren(iter, newItem.m_ItemList)
  else (no match on root)
    call AddMatchingChildren(iter, m_ItemList)

  Consideration of m_HierarchyHandling adds one branch
  */
  if (!treeIter->IsAtEnd()) // do nothing if the tree is empty!
  if ( m_Filter( treeIter->Get() ) )
  {
    m_Items->CreateElementAt(0) = Item::New( treeIter->Get(), this, 0, 0 ); // 0 = first item, 0 = no parent
    InvokeEvent( TreeFilterItemAddedEvent(m_Items->ElementAt(0)) );

    if ( m_HierarchyHandling == DataTreeFilter::PRESERVE_HIERARCHY )
    {
      AddMatchingChildren( treeIter,
                          m_Items->ElementAt(0)->m_Children,
                          m_Items->ElementAt(0).GetPointer(),
                          false );
    }
    else
    {
      AddMatchingChildren( treeIter, m_Items, 0, false );
    }
  }
  else
  {
    AddMatchingChildren( treeIter, m_Items, 0, false );
  }
  
  delete treeIter; // release data tree iterator
  InvokeEvent( TreeFilterUpdateAllEvent() );
}
      

} // namespace mitk 

// vi: textwidth=90
