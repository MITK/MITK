#ifndef PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D
#define PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D

#include "mitkCommon.h"
#include "BaseProperty.h"

namespace mitk {

//##ModelId=3E031F200392
class PropertyList : public itk::Object
{
   public:
	/** Standard class typedefs. */
	typedef PropertyList             Self;
	typedef itk::Object              Superclass;
	typedef itk::SmartPointer<Self>  Pointer;
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(PropertyList,itk::Object);
  public:
    //##ModelId=3D6A0E9E0029
    mitk::BaseProperty GetProperty(const char *propertyName);

    //##ModelId=3D78B966005F
    bool AddProperty(BaseProperty* property);

    //##ModelId=3D6A0F0B00BC
    BaseProperty properties;

};

} // namespace mitk



#endif /* PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D */
