#ifndef DATATREENODE_H_HEADER_INCLUDED_C1E14338
#define DATATREENODE_H_HEADER_INCLUDED_C1E14338

#include "mitkCommon.h"
#include "ImageSource.h"

namespace mitk {

class Mapper;

//##ModelId=3E031E2C0143
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
    mitk::Mapper* GetMapper();

  protected:
    //##ModelId=3D6A0F8C0202
	  mitk::Mapper* mappers;

};

} // namespace mitk



#endif /* DATATREENODE_H_HEADER_INCLUDED_C1E14338 */
