#ifndef DATATREENODE_H_HEADER_INCLUDED_C1E14338
#define DATATREENODE_H_HEADER_INCLUDED_C1E14338

#include "mitkCommon.h"
#include "ImageSource.h"
#include "Mapper.h"

namespace mitk {

//##ModelId=3E031E2C0143
class DataTreeNode : public ImageSource
{
  public:
	/** Standard class typedefs. */
	typedef DataTreeNode       Self;
	typedef ImageSource        Superclass;
	typedef itk::SmartPointer<Self>  Pointer;
	typedef itk::SmartPointer<const Self>  ConstPointer;
	  
	/** Run-time type information (and related methods). */
	itkTypeMacro(DataTreeNode,ImageSource);

    itkNewMacro(Self);  

	//##ModelId=3D6A0E8C02CC
    mitk::Mapper* GetMapper();

  protected:
    //##ModelId=3D6A0F8C0202
    Mapper mappers;

};

} // namespace mitk



#endif /* DATATREENODE_H_HEADER_INCLUDED_C1E14338 */
