#include "mitkLevelWindowProperty.h"


//##ModelId=3ED953090121
mitk::LevelWindowProperty::LevelWindowProperty()
{
}

//##ModelId=3EF198D9012D
mitk::LevelWindowProperty::LevelWindowProperty(const mitk::LevelWindow &levWin)
{
    SetLevelWindow(levWin);
}

//##ModelId=3ED953090122
mitk::LevelWindowProperty::~LevelWindowProperty()
{
}

//##ModelId=3ED953090124
bool mitk::LevelWindowProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_LevWin==m_LevWin;
}

//##ModelId=3ED953090133
const mitk::LevelWindow & mitk::LevelWindowProperty::GetLevelWindow() const
{
    return m_LevWin;
}

//##ModelId=3ED953090135
void mitk::LevelWindowProperty::SetLevelWindow(const mitk::LevelWindow &levWin)
{
    if(m_LevWin != levWin)
    {
        m_LevWin = levWin;
        Modified();
    }
}
