#include "Mapper.h"
#include "DataTreeNode.h"

//##ModelId=3E3C320001E4
//void mitk::Mapper::SetDisplaySizeInDisplayUnits(unsigned int widthInDisplayUnits, unsigned int heightInDisplayUnits)
//{
//    m_DisplaySizeInDisplayUnits.set(widthInDisplayUnits, heightInDisplayUnits); 
//}
//
//##ModelId=3E3C337E0162
mitk::Mapper::Mapper() //: m_DisplaySizeInDisplayUnits(10.0, 10.0)
{
	// Modify superclass default values, can be overridden by subclasses
	this->SetNumberOfRequiredInputs(1);
}


//##ModelId=3E3C337E019E
mitk::Mapper::~Mapper()
{
}

void mitk::Mapper::SetInput(const mitk::DataTreeNode* data)
{
    this->ProcessObject::SetNthInput(0,
        const_cast< mitk::DataTreeNode * >( data ) );
}

//##ModelId=3E860B9A0378
mitk::BaseData* mitk::Mapper::GetData() const
{
    return static_cast<const mitk::DataTreeNode *> (const_cast<mitk::Mapper*>(this)->ProcessObject::GetInput(0))->GetData();
}

