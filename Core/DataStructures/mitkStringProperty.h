#ifndef MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491
#define MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491

#include "mitkCommon.h"
#include "BaseProperty.h"
#include <string>

namespace mitk {

//##ModelId=3E3FDF05028B
class StringProperty : public BaseProperty
{
  protected:
    //##ModelId=3E3FDF21017D
    std::string m_String;

public:
    //##ModelId=3E3FF04F005F
	StringProperty( const char* string );
	itkGetStringMacro(String)
	itkSetStringMacro(String)

    //##ModelId=3E3FF04F00E1
	virtual bool operator==(const BaseProperty& property ) const;
};

} // namespace mitk

#endif /* MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491 */
