#include "mitkLookupTableProperty.h"


//##ModelId=3ED953090121
mitk::LookupTableProperty::LookupTableProperty()
{
}

//##ModelId=3EF198D9012D
mitk::LookupTableProperty::LookupTableProperty(const mitk::LookupTable &aLookupTable)
{
    SetLookupTable(aLookupTable);
}

//##ModelId=3ED953090122
mitk::LookupTableProperty::~LookupTableProperty()
{
}

//##ModelId=3ED953090124
bool mitk::LookupTableProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return *(other->m_LookupTable)==*m_LookupTable;
}

//##ModelId=3ED953090133
mitk::LookupTable & mitk::LookupTableProperty::GetLookupTable() 
{
    return *m_LookupTable;
}

//##ModelId=3ED953090135
void mitk::LookupTableProperty::SetLookupTable(const mitk::LookupTable &aLookupTable)
{
    if(*m_LookupTable != aLookupTable)
    {
        *m_LookupTable = aLookupTable;
        Modified();
    }
}
