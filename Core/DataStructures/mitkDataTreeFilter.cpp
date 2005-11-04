#include <itkCommand.h>
#include <mitkDataTreeFilter.h>
#include <mitkDataTree.h>
#include <mitkPropertyManager.h>

namespace mitk
{

//------ Some common filter functions ----------------------------------------------------

/// default filter, lets everything except NULL pointers pass
static bool IsDataTreeNode(mitk::DataTreeNode* node)
{
  return ( node != 0 );
}

//------ BasePropertyAccessor ------------------------------------------------------------
  
DataTreeFilter::BasePropertyAccessor::BasePropertyAccessor(mitk::BaseProperty* property, bool editable)
: m_Editable(editable),
  m_Property(property)
{
  // nothing to do
}

bool DataTreeFilter::BasePropertyAccessor::IsEditable() const
{
  return m_Editable;
}

DataTreeFilter::BasePropertyAccessor::operator const mitk::BaseProperty*()
{
  // const access always permitted
  return m_Property;
}

DataTreeFilter::BasePropertyAccessor::operator mitk::BaseProperty*()
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

DataTreeFilter::Item::Pointer DataTreeFilter::Item::New(mitk::DataTreeNode* node, DataTreeFilter* treefilter, 
                                                        int index, const Item* parent)
{
  // from itkNewMacro()
  Pointer smartPtr;
  Item* rawPtr = new Item(node, treefilter, index, parent);
  smartPtr = rawPtr;
  rawPtr->UnRegister();
  return smartPtr;
}

DataTreeFilter::Item::Item(mitk::DataTreeNode* node, DataTreeFilter* treefilter, 
                           int index, const Item* parent)
: m_Index(index),
  m_Parent(parent),
  m_Children(NULL),
  m_TreeFilter(treefilter),
  m_Node(node),
  m_Selected(false)
{
  m_Children = DataTreeFilter::ItemList::New();
}

DataTreeFilter::Item::~Item()
{
  // remove selection
  m_TreeFilter->m_SelectedItems.erase(this);
  // remove map entry
  m_TreeFilter->m_Item.erase(m_Node);
}

DataTreeFilter::BasePropertyAccessor DataTreeFilter::Item::GetProperty(const std::string& key) const
{
  mitk::BaseProperty* prop = m_Node->GetProperty(key.c_str(), m_TreeFilter->m_Renderer);
  if ( std::find( m_TreeFilter->m_VisibleProperties.begin(), m_TreeFilter->m_VisibleProperties.end(), key ) 
       == m_TreeFilter->m_VisibleProperties.end() )
  {
    // key not marked visible
    return BasePropertyAccessor(0, false);
  }

  if ( !prop ) // property not found in list (or "disabled")
  {
    mitk::BaseProperty::Pointer defaultProp = // create a default property
      mitk::PropertyManager::GetInstance()->CreateDefaultProperty(key.c_str());

    if ( defaultProp.IsNotNull() )
    {
      m_Node->SetProperty( key.c_str(), defaultProp );
      prop = defaultProp;
    }
  }

  // visible. determine whether property may be edited
  bool editable = 
    std::find(m_TreeFilter->m_EditableProperties.begin(), m_TreeFilter->m_EditableProperties.end(), key)
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
      if ( m_TreeFilter->m_SelectionMode == mitk::DataTreeFilter::SINGLE_SELECT )
      { // deselect last selected item
        m_TreeFilter->m_SelectedItems.erase( m_TreeFilter->m_LastSelectedItem.GetPointer() );
      }
      
      if ( m_TreeFilter->m_LastSelectedItem.IsNotNull() ) // remember this item as most recently selected
        m_TreeFilter->m_LastSelectedItem = this;

      m_TreeFilter->m_SelectedItems.insert(this);
    }
    else
    {
      m_TreeFilter->m_SelectedItems.erase(this);
    }
  }
}

const DataTreeFilter::Item* DataTreeFilter::Item::GetParent() const
{
  return m_Parent;
}

//------ DataTreeFilter ------------------------------------------------------------------

// smart pointer constructor
DataTreeFilter::Pointer DataTreeFilter::New(mitk::DataTree* datatree)
{
  // from itkNewMacro()
  Pointer smartPtr;
  DataTreeFilter* rawPtr = new DataTreeFilter(datatree);
  smartPtr = rawPtr;
  rawPtr->UnRegister();
  return smartPtr;
}

// real constructor (protected)
DataTreeFilter::DataTreeFilter(mitk::DataTree* datatree)
: m_Filter(NULL),
  m_DataTree(datatree),
  m_HierarchyHandling(mitk::DataTreeFilter::PRESERVE_HIERARCHY),
  m_SelectionMode(mitk::DataTreeFilter::MULTI_SELECT),
  m_Renderer(NULL),
  m_TreeChangeConnection(0),
  m_TreeAddConnection(0),
  m_TreeRemoveConnection(0)
{
  SetFilter( &mitk::IsDataTreeNode );
  m_Items = ItemList::New(); // create an empty list

  // connect tree's modified to my GenerateModelFromTree() 
  /**/
  {
  itk::ReceptorMemberCommand<mitk::DataTreeFilter>::Pointer command = itk::ReceptorMemberCommand<mitk::DataTreeFilter>::New();
  command->SetCallbackFunction(this, &DataTreeFilter::TreeChange);
  m_TreeChangeConnection = m_DataTree->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
  }
  /**/

  // connect tree's add to my GenerateModelFromTree()
  {
  itk::ReceptorMemberCommand<mitk::DataTreeFilter>::Pointer command = itk::ReceptorMemberCommand<mitk::DataTreeFilter>::New();
  command->SetCallbackFunction(this, &DataTreeFilter::TreeAdd);
  m_TreeAddConnection = m_DataTree->AddObserver(itk::TreeAddEvent<mitk::DataTreeBase>(), command);
  }
  
  // connect tree's remove to my GenerateModelFromTree()
  {
  itk::ReceptorMemberCommand<mitk::DataTreeFilter>::Pointer command = itk::ReceptorMemberCommand<mitk::DataTreeFilter>::New();
  command->SetCallbackFunction(this, &DataTreeFilter::TreeRemove);
  m_TreeRemoveConnection = m_DataTree->AddObserver(itk::TreeRemoveEvent<mitk::DataTreeBase>(), command);
  }
  
}

DataTreeFilter::~DataTreeFilter()
{
  // remove this as observer from the data tree
  if (m_TreeChangeConnection) m_DataTree->RemoveObserver( m_TreeChangeConnection );
  if (m_TreeAddConnection)    m_DataTree->RemoveObserver( m_TreeAddConnection );
  if (m_TreeRemoveConnection) m_DataTree->RemoveObserver( m_TreeRemoveConnection );
}

void DataTreeFilter::SetPropertiesLabels(const PropertyList labels)
{
  m_PropertyLabels = labels;
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetPropertiesLabels() const
{
  return m_PropertyLabels;
}

void DataTreeFilter::SetVisibleProperties(const PropertyList keys)
{
  m_VisibleProperties = keys;
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetVisibleProperties() const
{
  return m_VisibleProperties;
}
      
void DataTreeFilter::SetEditableProperties(const PropertyList keys)
{
  m_EditableProperties = keys;
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetEditableProperties() const
{
  return m_EditableProperties;
}

void DataTreeFilter::SetRenderer(const mitk::BaseRenderer* renderer)
{
  m_Renderer = renderer;
}
      
const mitk::BaseRenderer* DataTreeFilter::GetRenderer() const
{
  return m_Renderer;
}

void DataTreeFilter::SetFilter(FilterFunctionPointer filter)
{
  if (m_Filter == filter) return;
  
  if (filter)
    m_Filter = filter;
  else
    m_Filter = &mitk::IsDataTreeNode;

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
      
void DataTreeFilter::SelectItem(const Item* item, bool selected)
{
  // Ich const_caste mir die Welt, wie sie mir gefällt :-)
  // Aber die Items gehören ja dem Filter, deshalb ist das ok
  const_cast<Item*>(item) -> SetSelected(selected);
}

void DataTreeFilter::TreeChange(const itk::EventObject& e)
{
  if ( typeid(e) != typeid(itk::TreeChangeEvent<mitk::DataTreeBase>) ) return;
  
  // if event.GetChangePosition()->GetNode() == NULL, then something was removed (while in ~DataTree()) by setting it to NULL
  const itk::TreeChangeEvent<mitk::DataTreeBase>& event( static_cast<const itk::TreeChangeEvent<mitk::DataTreeBase>&>(e) );
  mitk::DataTreeIteratorBase* treePosition = const_cast<mitk::DataTreeIteratorBase*>(&(event.GetChangePosition()));

  if ( treePosition->Get().IsNull() ) return; // TODO this special case is used only in
                                              // ~DataTree(). If used anywhere else, this
                                              // line here has to be removed. But probably
                                              // nobody should set a node to NULL

  GenerateModelFromTree();
}

/// @todo Could TreeAdd be implemented more intelligent?
void DataTreeFilter::TreeAdd(const itk::EventObject& e)
{
  /// Regenerate item tree only from the changed position on (when hierarchy is preserved,
  /// otherwise rebuild everything)
  const itk::TreeAddEvent<mitk::DataTreeBase>& event( static_cast<const itk::TreeAddEvent<mitk::DataTreeBase>&>(e) );
  mitk::DataTreeIteratorBase* treePosition = const_cast<mitk::DataTreeIteratorBase*>(&(event.GetChangePosition()));
    
  mitk::DataTreeNode* newNode = treePosition->Get();

  if ( !m_Filter(newNode) ) return; // if filter does not match, then nothing has to be done

  if ( m_HierarchyHandling == DataTreeFilter::FLATTEN_HIERARCHY )
  {  // flat hierarchy -> rebuild all
    m_Items->clear();
    
    mitk::DataTreeIteratorBase* rootIter =  // get an iterator to the data tree
      new mitk::DataTreePreOrderIterator::PreOrderTreeIterator(m_DataTree);
    
    m_Items->CreateElementAt(0) = Item::New( rootIter->Get(), this, 0, 0 ); // 0 = first item, 0 = no parent
    m_Item[rootIter->Get()] = m_Items->ElementAt(0);
    InvokeEvent( mitk::TreeFilterItemAddedEvent(m_Items->ElementAt(0)) );

    AddMatchingChildren( rootIter , m_Items, 0);

    delete rootIter;
  }
  else
  {  // preserve hierarchy
    mitk::DataTreeIteratorBase* parentIter = 
      new mitk::DataTreePreOrderIterator::PreOrderTreeIterator(m_DataTree);

    m_Item[ treePosition->Get() ]->m_Children->clear(); // TODO remove-notify!
    
    AddMatchingChildren( treePosition, m_Item[ treePosition->Get() ]->m_Children, m_Item[ parentIter->Get() ] );
    
    delete parentIter;
  }
    
}

void DataTreeFilter::TreeRemove(const itk::EventObject&)
{
  // is called from either Remove() or RemoveChild(int)
  // no means to differentiate
  GenerateModelFromTree();
}

void DataTreeFilter::AddMatchingChildren(mitk::DataTreeIteratorBase* iter, ItemList* list,
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
      m_Item[iter->Get()] = list->ElementAt( newIndex );
      if (verbose) InvokeEvent( mitk::TreeFilterItemAddedEvent(m_Items->ElementAt(0)) );

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
  m_Items = ItemList::New(); // clear list (nice thanks to smart pointers)
  
  mitk::DataTreeIteratorBase* treeIter =  // get an iterator to the data tree
    new mitk::DataTreePreOrderIterator::PreOrderTreeIterator(m_DataTree);

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
    m_Item[treeIter->Get()] = m_Items->ElementAt(0);
    InvokeEvent( mitk::TreeFilterItemAddedEvent(m_Items->ElementAt(0)) );

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

  InvokeEvent( mitk::TreeFilterUpdateAllEvent() );
}
      

} // namespace mitk 

// vi: textwidth=90
