#include <itkCommand.h>
#include <mitkDataTreeFilter.h>
#include <mitkDataTreeFilterEvents.h>
#include <mitkDataTree.h>
#include <mitkDataTreeHelper.h>
#include <mitkPropertyManager.h>
#include <mitkImage.h>
#include <mitkDataTreeFilterFunctions.h>

#include <set>

#ifndef NDEBUG
#include <ostream>
  #define DEBUG_STATE           if (m_DEBUG) PrintStateForDebug(std::cout);

  #define DEBUG_MSG_STATE(MSG)  if (m_DEBUG) { \
                                              std::cout << "========================================" << std::endl; \
                                              std::cout << MSG << std::endl; \
                                              PrintStateForDebug(std::cout); \
                                             }

  #define DEBUG_MSG(MSG)        if (m_DEBUG) std::cout << MSG << std::endl;
#else
  #define DEBUG_STATE
  #define DEBUG_MSG_STATE(MSG)
  #define DEBUG_MSG(MSG)
#endif

namespace mitk
{

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
  if (m_TreeFilter->m_LastSelectedItem == this)
    m_TreeFilter->m_LastSelectedItem = NULL;
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

// smart pointer constructor
DataTreeFilter::Pointer DataTreeFilter::New(DataTreeIteratorBase* iterator)
{
  // from itkNewMacro()
  Pointer smartPtr;
  DataTreeFilter* rawPtr = new DataTreeFilter(iterator->GetTree());
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
  m_TreeChangeConnection(0),
  m_LastSelectedItem(NULL),
  m_AutoUpdate(false),
  m_SelectMostRecentItemMode(false)
#ifndef NDEBUG
  , m_DEBUG(false)
#endif
{
  //SetFilter( &IsDataTreeNode );
  SetFilter( IsBaseDataType<Image>() );
  m_VisibleProperties.push_back("name");
  
  m_Items = ItemList::New(); // create an empty list

  SetAutoUpdate( true );
 
  GenerateModelFromTree();
}
 
DataTreeFilter::~DataTreeFilter()
{
  DisconnectTreeEvents();
 
  InvokeEvent( TreeFilterRemoveAllEvent() );

  m_Items->clear();
  m_Item.clear(); // should already BE clear by now
  m_SelectedItems.clear();

  if ( m_Filter ) delete m_Filter;
}

const DataTreeBase* DataTreeFilter::GetDataTree() const
{
  return m_DataTree.GetPointer();
}

void DataTreeFilter::ConnectTreeEvents()
{
  // connect tree notifications to member functions
  {
  itk::ReceptorMemberCommand<DataTreeFilter>::Pointer command = itk::ReceptorMemberCommand<DataTreeFilter>::New();
  command->SetCallbackFunction(this, &DataTreeFilter::TreeChange);
  m_TreeChangeConnection = m_DataTree->AddObserver(itk::TreeChangeEvent<DataTreeBase>(), command);
  }
}

void DataTreeFilter::DisconnectTreeEvents()
{
  // remove this as observer from the data tree
  m_DataTree->RemoveObserver( m_TreeChangeConnection );
}

bool DataTreeFilter::GetSelectMostRecentItemMode()
{
  return m_SelectMostRecentItemMode;
}

void DataTreeFilter::SetSelectMostRecentItemMode(bool automaticallySelectMostRecent)
{
  if (automaticallySelectMostRecent != m_SelectMostRecentItemMode)
  {
    m_SelectMostRecentItemMode = automaticallySelectMostRecent;
    GenerateModelFromTree();
  }
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

void DataTreeFilter::SetFilter(const DataTreeFilterFunction* filter)
{
  if (m_Filter == filter) return; //TODO check for class identity, not object identity
  
  if (filter)
  {
    delete m_Filter;
    m_Filter = filter->Clone();
  }
  else
  {
    delete m_Filter;
    m_Filter = new IsBaseDataType<Image>;
  }

  GenerateModelFromTree();
}

void DataTreeFilter::SetFilter(const DataTreeFilterFunction& filter)
{
  SetFilter(&filter);
}

void DataTreeFilter::SetDataStorageResultset(const mitk::DataStorage::SetOfObjects* rs)
{
  this->SetFilter(IsInResultSet(rs));
}

const DataTreeFilterFunction* DataTreeFilter::GetFilter() const
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

const DataTreeFilter::Item* DataTreeFilter::FindItem(const DataTreeNode* node, DataTreeFilter::ItemList* itemList) const
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

const DataTreeFilter::Item* DataTreeFilter::FindItem(const DataTreeNode* node) const
{
  return FindItem(node, m_Items);
}
      
void DataTreeFilter::SelectItem(const Item* item, bool selected)
{
  // Ich const_caste mir die Welt, wie sie mir gefaellt :-)
  // Aber die Items gehoeren ja dem Filter, deshalb ist das ok
  const_cast<Item*>(item) -> SetSelected(selected);
} 

void DataTreeFilter::UnselectAll()
{
  while ( m_SelectedItems.size() != 0 ) // for all selected items
  {
    (*m_SelectedItems.begin())->SetSelected(false);
  }
}

void DataTreeFilter::DeleteSelectedItems()
{
  while ( m_SelectedItems.size() != 0 ) // for all selected items
  {
    DataTreeIteratorClone toNode( DataTreeHelper::FindIteratorToNode(m_DataTree, (*(m_SelectedItems.begin()))->GetNode()) );
    toNode->Disconnect(); // remove just this one item (Remove() would kill all children, too)
  }
}

void DataTreeFilter::DeleteSelectedItemsAndSubItems()
{
  while ( m_SelectedItems.size() != 0 ) // for all selected items
  {
    DataTreeIteratorClone toNode( DataTreeHelper::FindIteratorToNode(m_DataTree, (*(m_SelectedItems.begin()))->GetNode()) );
    toNode->Remove(); 
  }
}

void DataTreeFilter::TreeChange( const itk::EventObject& itkNotUsed( e ) )
{
  //if ( typeid(e) != typeid(itk::TreeChangeEvent<DataTreeBase>) ) return;
  DEBUG_MSG_STATE("Before TreeChangeEvent")
 

  GenerateModelFromTree();
  DEBUG_MSG_STATE("After TreeChangeEvent")
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
    if ( (*m_Filter)( iter->Get() ) )
    {
      unsigned int newIndex = list->Size();
      Item::Pointer newItem = Item::New( iter->Get(), this, newIndex, parent );
      list->CreateElementAt( newIndex ) = newItem;

      if ( newItem->GetNode() && newItem->GetNode()->GetMTime() > m_LargestMTime )
      {
        m_LargestMTime = newItem->GetNode()->GetMTime();
        m_LargestMTimeItem = newItem;
      }

    
      // restore selection state for this new item
      for (DataTreeNodeSet::iterator siter = m_LastSelectedNodes.begin();
           siter != m_LastSelectedNodes.end();
           ++siter)
      {
        if ( *siter == newItem->GetNode() )
        {
          m_SelectedItems.insert(newItem);
          newItem->m_Selected = true;
        }
      }

      if ( m_LastSelectedNode == newItem->GetNode() )
      {
        m_LastSelectedItem = newItem;
        newItem->m_Selected = true; // probably not neccessary
      }

      if (verbose) InvokeEvent( TreeFilterItemAddedEvent(list->ElementAt(newIndex)) );

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

void DataTreeFilter::StoreCurrentSelectionState()
{
  // save current selection
  if (m_LastSelectedItem)
    m_LastSelectedNode = m_LastSelectedItem->GetNode();
  else
    m_LastSelectedNode = NULL;

  m_LastSelectedNodes.clear();
  for (ItemSet::iterator iter = m_SelectedItems.begin();
       iter != m_SelectedItems.end();
       ++iter)
  {
    m_LastSelectedNodes.insert( (*iter)->GetNode() );
  }
}

void DataTreeFilter::GenerateModelFromTree()
{
  DEBUG_MSG_STATE("Before GenerateModel")

  StoreCurrentSelectionState();

  // remember all nodes we have now
  DataTreeNodeSet oldNodes;
  for ( TreeNodeItemMap::iterator iterOldNodes = m_Item.begin();
        iterOldNodes != m_Item.end();
        ++iterOldNodes )

  {
    oldNodes.insert( iterOldNodes->first );
  }
  
  m_Items = ItemList::New(); // clear list (nice thanks to smart pointers)

  DataTreeIteratorBase* treeIter =  // get an iterator to the data tree
    new DataTreePreOrderIterator(m_DataTree);
  
  m_LargestMTime = 0;
  m_LargestMTimeItem = NULL;

  /*
  if root matches
    create an Item for root
    call AddMatchingChildren(iter, newItem.m_ItemList)
  else (no match on root)
    call AddMatchingChildren(iter, m_ItemList)

  Consideration of m_HierarchyHandling adds one branch
  */
  if (!treeIter->IsAtEnd()) // do nothing if the tree is empty!
  if ( (*m_Filter)( treeIter->Get() ) )
  {
    m_Items->CreateElementAt(0) = Item::New( treeIter->Get(), this, 0, 0 ); // 0 = first item, 0 = no parent
    Item* newItem = m_Items->ElementAt(0); 

    if ( newItem->GetNode() && newItem->GetNode()->GetMTime() > m_LargestMTime )
    {
      m_LargestMTime = newItem->GetNode()->GetMTime();
      m_LargestMTimeItem = newItem;
    }

    // restore selection state for this new item
    for (DataTreeNodeSet::iterator siter = m_LastSelectedNodes.begin();
         siter != m_LastSelectedNodes.end();
         ++siter)
    {
      if ( *siter == newItem->GetNode() )
      {
        m_SelectedItems.insert(newItem);
        newItem->m_Selected = true;
      }
    }

    if ( m_LastSelectedNode == newItem->GetNode() )
    {
      m_LastSelectedItem = newItem;
      newItem->m_Selected = true; // probably not neccessary
    }

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

  DEBUG_MSG_STATE("After GenerateModel - before selection of most recent item")

  if ( m_SelectMostRecentItemMode )
  for ( TreeNodeItemMap::iterator iterNewNodes = m_Item.begin();
        iterNewNodes != m_Item.end();
        ++iterNewNodes )

  {
    if ( oldNodes.find( iterNewNodes->first ) == oldNodes.end() )
    {
      // found a completely new item, automatically select that
      iterNewNodes->second->SetSelected( true );
      DEBUG_MSG_STATE("After GenerateModel - after selection of most recent item")
      return;
    }
  }
   
  DEBUG_MSG_STATE("After GenerateModel - after selection of most recent item")
}

void DataTreeFilter::SetAutoUpdate(bool autoUpdatesEnabled)
{
  if ( autoUpdatesEnabled != m_AutoUpdate )
  {
    if ( autoUpdatesEnabled )
    {
      ConnectTreeEvents();
    }
    else
    {
      DisconnectTreeEvents();
    }

    m_AutoUpdate = autoUpdatesEnabled;
  }
}

void DataTreeFilter::Update()
{
  DEBUG_MSG_STATE("Before Update")

  GenerateModelFromTree();
  DEBUG_MSG_STATE("After Update")
}

#ifndef NDEBUG
void DataTreeFilter::PrintStateForDebug(std::ostream& out)
{
  out << "Current state: " 
      << m_Items->size() << " items, "
      << m_SelectedItems.size() << " selected, "
      << m_Item.size() << " mapped to nodes, "
      << "filter at " << (void*)m_Filter << ","
      << std::endl;
  
  ItemList::iterator listIter;
  for ( listIter = m_Items->begin(); listIter != m_Items->end(); ++listIter )
  {
    out << (void*)(*listIter) << ": ";
    if (*listIter)
    {
      std::string name;
      (*listIter)->GetNode()->GetName(name);
      out << name;
      if ((*listIter)->IsSelected()) 
        out << ", is selected ";
      if ( (*listIter)->HasChildren() )
        out << ", has children";
      out << std::endl;
    }
  }
  out << std::endl;
}
#endif

} // namespace mitk 

// vi: textwidth=90

