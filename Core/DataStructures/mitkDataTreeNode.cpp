#include "DataTreeNode.h"
#include "MapperFactory.h"

//##ModelId=3D6A0E8C02CC
mitk::Mapper* mitk::DataTreeNode::GetMapper(MapperSlotId id) const
{
// TODO

   if (mappers[id] == NULL) {

     mappers[id] = MapperFactory::CreateMapper(GetData(),id);

   }

   // mappers[id]->AddInput(GetData());

   return mappers[id];

  

   //  return NULL;

}

//##ModelId=3E32C49D00A8
mitk::BaseData* mitk::DataTreeNode::GetData() const
{
  return m_Data;
}



//##ModelId=3E33F4E4025B
mitk::DataTreeNode::SetData(mitk::BaseData* baseData)
{
    m_Data=baseData;
}

//##ModelId=3E33F5D702AA
mitk::DataTreeNode::DataTreeNode()
{
    memset(mappers, 0, sizeof(mappers)); 
}


//##ModelId=3E33F5D702D3
mitk::DataTreeNode::~DataTreeNode()
{

}

//##ModelId=3E33F5D7032D
mitk::DataTreeNode& mitk::DataTreeNode::operator=(const DataTreeNode& right)
{
    mitk::DataTreeNode* node=mitk::DataTreeNode::New();
    node->SetData(right.GetData());
    return *node;
}

mitk::DataTreeNode& mitk::DataTreeNode::operator=(mitk::BaseData* right)
{
    mitk::DataTreeNode* node=mitk::DataTreeNode::New();
    node->SetData(right);
    return *node;
}

//##ModelId=3E33F5D703D7
MBI_STD::istream& mitk::operator>>( MBI_STD::istream& i, DataTreeNode::Pointer& at )
{
    return i;
}

//##ModelId=3E33F5D8007B
MBI_STD::ostream& mitk::operator<<( MBI_STD::ostream& o, DataTreeNode::Pointer& t)
{
    return o;
}

