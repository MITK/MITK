#include "DataTreeNode.h"
#include "MapperFactory.h"

//##ModelId=3D6A0E8C02CC
mitk::Mapper* mitk::DataTreeNode::GetMapper(MapperSlotId id) const
{
// TODO

   if (mappers[id] == NULL) {

     mappers[id] = MapperFactory::CreateMapper(const_cast<DataTreeNode*>(this),id);

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
void mitk::DataTreeNode::SetData(mitk::BaseData* baseData)
{
    m_Data=baseData;
}

//##ModelId=3E33F5D702AA
mitk::DataTreeNode::DataTreeNode() : m_Data(NULL)
{
    memset(mappers, 0, sizeof(mappers)); 

    m_PropertyList = PropertyList::New();
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

//##ModelId=3E69331903C9
void mitk::DataTreeNode::SetMapper(MapperSlotId id, mitk::Mapper* mapper)
{
    mappers[id] = mapper;

    if (mapper!=NULL)
        mapper->SetInput(this);
}

//##ModelId=3E860A5C0032
void mitk::DataTreeNode::UpdateOutputInformation()
{
	if (this->GetSource())
	{
		this->GetSource()->UpdateOutputInformation();
	}
}

//##ModelId=3E860A5E011B
void mitk::DataTreeNode::SetRequestedRegionToLargestPossibleRegion()
{
}

//##ModelId=3E860A5F03D9
bool mitk::DataTreeNode::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}

//##ModelId=3E860A620080
bool mitk::DataTreeNode::VerifyRequestedRegion()
{
    return GetData()!=NULL;
}

//##ModelId=3E860A640156
void mitk::DataTreeNode::SetRequestedRegion(itk::DataObject *data)
{
}

//##ModelId=3E860A6601DB
void mitk::DataTreeNode::CopyInformation(const itk::DataObject *data)
{
}
//##ModelId=3E3FE0420273
mitk::PropertyList::Pointer mitk::DataTreeNode::GetPropertyList() const
{
    return m_PropertyList;
}

