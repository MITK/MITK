#include "mitkLevelWindowProperty.h"


//##ModelId=3E86AABB030D
mitk::LevelWindowProperty::LevelWindowProperty()
{

}

//##ModelId=3E86A345014E
mitk::LevelWindowProperty::~LevelWindowProperty()
{
}

//##ModelId=3E86A35F000B
bool mitk::LevelWindowProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_LevWin==m_LevWin;
}

//##ModelId=3E86AABB0371
mitk::LevelWindow & mitk::LevelWindowProperty::GetLevelWindow()
{
    return m_LevWin;
}

//##ModelId=3E86AABB0399
void mitk::LevelWindowProperty::SetLevelWindow(mitk::LevelWindow &levWin )
{
    if(m_LevWin != levWin)
    {
        m_LevWin = levWin;
        Modified();
    }
}
