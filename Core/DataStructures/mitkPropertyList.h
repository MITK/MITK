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
    //##ModelId=3E38FF6A022F
	typedef PropertyList             Self;
    //##ModelId=3E38FF6A0261
	typedef itk::Object              Superclass;
    //##ModelId=3E38FF6A0289
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E38FF6A02B1
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(PropertyList,itk::Object);
  public:
    //##ModelId=3D6A0E9E0029
    mitk::BaseProperty* GetProperty(const char *propertyName);

    //##ModelId=3D78B966005F
    bool AddProperty(BaseProperty* property);

    //##ModelId=3D6A0F0B00BC
    std::vector<BaseProperty::Pointer> properties;
  protected:
    //##ModelId=3E38FEFE0125
    PropertyList();


    //##ModelId=3E38FEFE0157
    virtual ~PropertyList();

};

} // namespace mitk



#endif /* PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D */
