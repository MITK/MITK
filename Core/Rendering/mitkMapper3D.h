#ifndef MAPPER3D_H_HEADER_INCLUDED_C1C517B9
#define MAPPER3D_H_HEADER_INCLUDED_C1C517B9

#include "mitkCommon.h"
#include "Mapper.h"

namespace mitk {

//##ModelId=3E3AA2F60390
class Mapper3D : public Mapper
{
public:
	/** Standard class typedefs. */
    //##ModelId=3E3AE1F20391
	typedef Mapper3D                 Self;
    //##ModelId=3E3AE1F203A5
	typedef Mapper                   Superclass;
    //##ModelId=3E3AE1F203C3
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E3AE1F203D7
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	//itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(Mapper3D,Mapper);

protected:
    //##ModelId=3E3AE1C5010C
    Mapper3D();

    //##ModelId=3E3AE1C5012A
    virtual ~Mapper3D();

};

} // namespace mitk



#endif /* MAPPER3D_H_HEADER_INCLUDED_C1C517B9 */
