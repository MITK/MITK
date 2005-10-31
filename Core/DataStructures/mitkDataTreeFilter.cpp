#include "mitkDataTreeFilter.h"
#include <itkCommand.h>
#include "mitkDataTree.h"

namespace mitk
{


//------ BasePropertyAccessor ------------------------------------------------------------
  
DataTreeFilter::BasePropertyAccessor::BasePropertyAccessor(mitk::BaseProperty& property, bool editable)
: m_Editable(editable),
  m_Property(property)
{
  // nothing to do
}

bool DataTreeFilter::BasePropertyAccessor::IsEditable() const
{
  return m_Editable;
}

DataTreeFilter::BasePropertyAccessor::operator const mitk::BaseProperty&()
{
  // return some const reference
}

DataTreeFilter::BasePropertyAccessor::operator mitk::BaseProperty&()
{
  // return some non-const reference
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

DataTreeFilter::BasePropertyAccessor& DataTreeFilter::Item::operator[](const std::string& key) const
{
}

const DataTreeFilter::ItemList* DataTreeFilter::Item::GetChildren() const
{
  return m_Children;
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
  m_Selected = selected;

  // notify TreeFilter, whether this item is now selected
  // ! notification can happen from SelectItem(Item, bool) of DataTreeFilter
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
  m_ObserverTag(0)
{
  // connect tree's modified to my GenerateModelFromTree()
  itk::ReceptorMemberCommand<mitk::DataTreeFilter>::Pointer command =
                                                          itk::ReceptorMemberCommand<mitk::DataTreeFilter>::New();
  
  command->SetCallbackFunction(this, &DataTreeFilter::TreeChanged);
  
  m_ObserverTag = m_DataTree->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
}

DataTreeFilter::~DataTreeFilter()
{
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
  m_Filter = filter;
}
      
const DataTreeFilter::FilterFunctionPointer DataTreeFilter::GetFilter() const
{
  return m_Filter;
}
      
void DataTreeFilter::SetSelectionMode(const SelectionMode selectionMode)
{
  m_SelectionMode = selectionMode;
}

DataTreeFilter::SelectionMode DataTreeFilter::GetSelectionMode() const
{
  return m_SelectionMode;
}
      
void DataTreeFilter::SetHierarchyHandling(const HierarchyHandling hierarchyHandling)
{
  m_HierarchyHandling = hierarchyHandling;
}

DataTreeFilter::HierarchyHandling DataTreeFilter::GetHierarchyHandling() const
{
  return m_HierarchyHandling;
}

const DataTreeFilter::ItemList* DataTreeFilter::GetItems() const
{
  return m_Items;
}
      
void DataTreeFilter::SelectItem(const Item* item, bool selected)
{
  const_cast<Item*>(item) -> SetSelected(selected);
}

void DataTreeFilter::TreeChanged(const itk::EventObject &)
{
  GenerateModelFromTree();
}

void DataTreeFilter::AddMatchingChildren(mitk::DataTreeIteratorBase* iter, ItemList* list, Item* parent)
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
  */
  int numChildren( iter->CountChildren() );

  for ( int child = 0; child < numChildren; ++child )
  {
    if ( m_Filter( iter->Get() ) )
    {
      unsigned int newIndex = list->Size();
      list->CreateElementAt( newIndex ) = Item::New( iter->Get(), this, newIndex, parent );
      iter->GoToChild(child);
      AddMatchingChildren( iter, 
                           list->ElementAt(newIndex)->m_Children,
                           list->ElementAt(newIndex).GetPointer() );
      iter->GoToParent();
    }
    else
    {
      iter->GoToChild(child);
      AddMatchingChildren( iter, list, parent );
      iter->GoToParent();
    }
  }
}

void DataTreeFilter::GenerateModelFromTree()
{
  if (!m_Filter); // TODO install default filter (always true)

  m_Items = ItemList::New(); // clear list (nice thanks to smart pointers)
  
  mitk::DataTreeIteratorBase* treeIter =  // get an iterator to the data tree
    new mitk::DataTreePreOrderIterator::PreOrderTreeIterator(m_DataTree);

  /*
  if root matches
    create an Item for root
    call AddMatchingChildren(iter, newItem.m_ItemList)
  else (no match on root)
    call AddMatchingChildren(iter, m_ItemList)
  */
  if ( m_Filter( treeIter->Get() ) )
  {
    m_Items->CreateElementAt(0) = Item::New( treeIter->Get(), this, 0, 0 ); // 0 = first item, 0 = no parent
    AddMatchingChildren( treeIter,
                         m_Items->ElementAt(0)->m_Children,
                         m_Items->ElementAt(0).GetPointer());
  }
  else
  {
    AddMatchingChildren( treeIter, m_Items, 0);
  }
  
  delete treeIter; // release data tree iterator
}
      

} // namespace mitk 

// vi: textwidth=90
