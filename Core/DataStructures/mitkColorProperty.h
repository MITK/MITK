#ifndef MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1
#define MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1

#include "mitkCommon.h"
#include "BaseProperty.h"
#include <itkRGBPixel.h>

namespace mitk {

//##ModelId=3E86AABB0082
//##Documentation
//## @brief Standard RGB color typedef (float)
//## 
//## Standard RGB color typedef to get rid of template argument (float).
typedef itk::RGBPixel< float > Color;

//##ModelId=3E86A20C00FD
//##Documentation
//## @brief RGB color property
class ColorProperty : public BaseProperty
{
protected:
    //##ModelId=3E86A3B00061
    mitk::Color m_Color;
public:
    mitkClassMacro(ColorProperty, BaseProperty);


    //##ModelId=3E86A3450130
    ColorProperty(const float color[3]);

    //##ModelId=3ED9530801BD
    ColorProperty(const mitk::Color & color);

    //##ModelId=3E86A345014E
    virtual ~ColorProperty();

    //##ModelId=3E86A35F000B
    virtual bool operator==(const BaseProperty& property) const;

    //##ModelId=3E86AABB0371
    const mitk::Color & GetColor() const;
    //##ModelId=3E86AABB0399
    void SetColor(const mitk::Color & color );
};

} // namespace mitk

#endif /* MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1 */
