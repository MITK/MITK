#include "mitkColorProperty.h"

//##ModelId=3E86A3450130
mitk::ColorProperty::ColorProperty(const float color[3]) : m_Color(color)
{

}

//##ModelId=3E86A3450130
mitk::ColorProperty::ColorProperty(const float red, const float green, const float blue)
{
  m_Color.Set(red, green, blue);
}

//##ModelId=3E86AABB030D
mitk::ColorProperty::ColorProperty(const mitk::Color & color) : m_Color(color)
{

}

//##ModelId=3E86A345014E
mitk::ColorProperty::~ColorProperty()
{
}

//##ModelId=3E86A35F000B
bool mitk::ColorProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_Color==m_Color;
}

//##ModelId=3E86AABB0371
const mitk::Color & mitk::ColorProperty::GetColor() const
{
    return m_Color;
}

//##ModelId=3E86AABB0399
void mitk::ColorProperty::SetColor(const mitk::Color & color )
{
    if(m_Color!=color)
    {
        m_Color = color;
        Modified();
    }
}
