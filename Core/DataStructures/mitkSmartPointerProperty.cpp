#include "mitkSmartPointerProperty.h"

//##ModelId=3ED94B4203C1
bool mitk::SmartPointerProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_SmartPointer==m_SmartPointer;
}

//##ModelId=3ED94B7500F2
mitk::SmartPointerProperty::SmartPointerProperty()
{
}

//##ModelId=3ED94B750111
mitk::SmartPointerProperty::~SmartPointerProperty()
{
}

//##ModelId=3ED952AD02B7
itk::Object::Pointer mitk::SmartPointerProperty::GetSmartPointer() const
{
    return m_SmartPointer;
}

//##ModelId=3ED952AD02F6
void mitk::SmartPointerProperty::SetSmartPointer(itk::Object* pointer)
{
    if(m_SmartPointer.GetPointer() != pointer)
    {
        m_SmartPointer = pointer;
        Modified();
    }
}
