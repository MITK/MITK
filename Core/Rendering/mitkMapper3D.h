#ifndef MAPPER3D_H_HEADER_INCLUDED_C1C517B9
#define MAPPER3D_H_HEADER_INCLUDED_C1C517B9

#include "mitkCommon.h"
#include "mitkMapper.h"
#include "mitkImageChannelSelector.h"

namespace mitk {

//##ModelId=3E3AA2F60390
//##Documentation
//## @brief Base class of all mappers for 3D display
//## @ingroup Mapper
class Mapper3D : public Mapper
{
public:
	mitkClassMacro(Mapper3D,Mapper);

	/** Method for creation through the object factory. */
	//itkNewMacro(Self);

protected:
    //##ModelId=3E3AE1C5010C
    Mapper3D();

    //##ModelId=3E3AE1C5012A
    virtual ~Mapper3D();

};

} // namespace mitk



#endif /* MAPPER3D_H_HEADER_INCLUDED_C1C517B9 */
