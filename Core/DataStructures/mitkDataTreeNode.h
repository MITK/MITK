#ifndef DATATREENODE_H_HEADER_INCLUDED_C1E14338
#define DATATREENODE_H_HEADER_INCLUDED_C1E14338

#include "ImageSource.h"
#include "BaseData.h"
#include "Mapper.h"

#ifdef MBI_NO_STD_NAMESPACE
	#define MBI_STD
	#include <iostream.h>
	#include <fstream.h>
#else
	#define MBI_STD std
	#include <iostream>
	#include <fstream>
#endif

#include "PropertyList.h"

namespace mitk {

//##ModelId=3E031E2C0143
//##Documentation
//## Contains the data (instance of BaseData) and a list of mappers, which can
//## draw the data.
class DataTreeNode : public BaseData
{
public:
    mitkClassMacro(DataTreeNode,BaseData);

	itkNewMacro(Self);  

	//##ModelId=3D6A0E8C02CC
	mitk::Mapper* GetMapper(MapperSlotId id) const;
	//##ModelId=3E32C49D00A8
	BaseData* GetData() const;
    //##ModelId=3E33F4E4025B
    virtual void SetData(mitk::BaseData* baseData);
    //##ModelId=3E33F5D7032D
    mitk::DataTreeNode& operator=(const DataTreeNode& right);

    //##ModelId=3E34248B02E0
    mitk::DataTreeNode& operator=(BaseData* right);
    //##ModelId=3E69331903C9
    virtual void SetMapper(MapperSlotId id, mitk::Mapper* mapper);
    //##ModelId=3E860A5C0032
    virtual void UpdateOutputInformation();

    //##ModelId=3E860A5E011B
    virtual void SetRequestedRegionToLargestPossibleRegion();

    //##ModelId=3E860A5F03D9
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

    //##ModelId=3E860A620080
    virtual bool VerifyRequestedRegion();

    //##ModelId=3E860A640156
    virtual void SetRequestedRegion(itk::DataObject *data);

    //##ModelId=3E860A6601DB
    virtual void CopyInformation(const itk::DataObject *data);
    //##ModelId=3E3FE0420273
    mitk::PropertyList::Pointer GetPropertyList() const;

protected:
    //##ModelId=3E33F5D702AA
    DataTreeNode();

    //##ModelId=3E33F5D702D3
    virtual ~DataTreeNode();

	//TODO: change to stl-vector
	//##ModelId=3D6A0F8C0202
    mutable mitk::Mapper::Pointer mappers[10];

	//##ModelId=3E32C49D0095
    BaseData::Pointer m_Data;

    //##ModelId=3E861B84033C
    PropertyList::Pointer m_PropertyList;
};


MBI_STD::istream& operator>>( MBI_STD::istream& i, DataTreeNode::Pointer& at );

MBI_STD::ostream& operator<<( MBI_STD::ostream& o, DataTreeNode::Pointer& t);

} // namespace mitk







#endif /* DATATREENODE_H_HEADER_INCLUDED_C1E14338 */

