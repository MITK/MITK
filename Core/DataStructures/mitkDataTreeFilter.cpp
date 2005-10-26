#include "mitkDataTreeFilter.h"

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

DataTreeFilter::Item::Item(mitk::DataTreeNode& node, DataTreeFilter& treefilter, 
                           int index, const Item& parent)
: m_Index(index),
  m_Parent(&parent),
  m_Children(),
  m_TreeFilter(treefilter),
  m_Node(node)
{
  // nothing to do
}

DataTreeFilter::Item::Item(mitk::DataTreeNode& node, DataTreeFilter& treefilter, 
                           int index)
: m_Index(index),
  m_Parent(NULL),
  m_Children(),
  m_TreeFilter(treefilter),
  m_Node(node)
{
  // nothing to do
}

DataTreeFilter::BasePropertyAccessor& DataTreeFilter::Item::operator[](const std::string& key) const
{
}
         
const DataTreeFilter::ItemList& DataTreeFilter::Item::GetChildren() const
{
}

int DataTreeFilter::Item::GetIndex() const
{
  return m_Index;
}

bool DataTreeFilter::Item::IsRoot() const
{
  return m_Parent == NULL;
}

const DataTreeFilter::Item& DataTreeFilter::Item::GetParent() const
{
  if (m_Parent)
  {
    return *m_Parent;
  }
  else
  { 
    throw HasNoParent();
  }
}

//------ RootItem ------------------------------------------------------------------------
DataTreeFilter::RootItem::RootItem(mitk::DataTreeNode& node, DataTreeFilter& treefilter, 
                                   int index)
: Item(node, treefilter, index)
{
  // nothing to do
}

//------ DataTreeFilter ------------------------------------------------------------------
     
DataTreeFilter::Pointer DataTreeFilter::New(mitk::DataTree& datatree)
{
  Pointer smartPtr;
  DataTreeFilter* rawPtr = new DataTreeFilter(datatree);
  smartPtr = rawPtr;
  rawPtr->UnRegister();
  return smartPtr;
}

DataTreeFilter::DataTreeFilter(mitk::DataTree& datatree)
: m_Items(),
  m_Filter(NULL),
  m_DataTree(datatree)
{
}

DataTreeFilter::~DataTreeFilter()
{
}

void DataTreeFilter::SetVisibleProperties(const PropertyList keys)
{
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetVisibleProperties() const
{
}
      
void DataTreeFilter::SetPropertiesLabels(const PropertyList labels)
{
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetPropertiesLabels() const
{
}

void DataTreeFilter::SetEditableProperties(const PropertyList keys)
{
}

const DataTreeFilter::PropertyList& DataTreeFilter::GetEditableProperties() const
{
}

void DataTreeFilter::SetRenderer(const mitk::BaseRenderer* renderer)
{
}

void DataTreeFilter::SetFilter(FilterFunctionPointer filter)
{
}

const DataTreeFilter::RootItemList& DataTreeFilter::GetItems() const
{
}
      

} // namespace mitk


// vi: textwidth=90
