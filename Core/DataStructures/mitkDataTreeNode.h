#ifndef DATATREENODE_H_HEADER_INCLUDED_C1E14338
#define DATATREENODE_H_HEADER_INCLUDED_C1E14338

#include "ImageSource.h"
#include "BaseData.h"
#include "Mapper.h"

#ifdef MBI_NO_STD_NAMESPACE
	#define MBI_STD
	#include <iostream.h>
	#include <fstream.h>
#include "BaseRenderer.h"
#else
	#define MBI_STD std
	#include <iostream>
	#include <fstream>
#endif

namespace mitk {

//##ModelId=3E031E2C0143
//##Documentation
//## Contains the data (instance of BaseData) and a list of mappers, which can
//## draw the data.
class DataTreeNode : public ImageSource
{
public:
	/** Standard class typedefs. */
	//##ModelId=3E1EB44101C3
	typedef DataTreeNode       Self;
	//##ModelId=3E1EB44101E1
	typedef ImageSource        Superclass;
	//##ModelId=3E1EB44101F5
	typedef itk::SmartPointer<Self>  Pointer;
	//##ModelId=3E1EB44101FF
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Run-time type information (and related methods). */
	itkTypeMacro(DataTreeNode,ImageSource);

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


protected:
    //##ModelId=3E33F5D702AA
    DataTreeNode();

    //##ModelId=3E33F5D702D3
    virtual ~DataTreeNode();

	//TODO: change to stl-vector
	//##ModelId=3D6A0F8C0202
    mutable mitk::Mapper::Pointer mappers[10];

	//##ModelId=3E32C49D0095
	BaseData* m_Data;
};


MBI_STD::istream& operator>>( MBI_STD::istream& i, DataTreeNode::Pointer& at );

MBI_STD::ostream& operator<<( MBI_STD::ostream& o, DataTreeNode::Pointer& t);


} // namespace mitk







#endif /* DATATREENODE_H_HEADER_INCLUDED_C1E14338 */

