#ifndef MITK_LEVEL_WINDOW_PROPERTY_H_
#define MITK_LEVEL_WINDOW_PROPERTY_H_

#include "mitkCommon.h"
#include "BaseProperty.h"
#include "LevelWindow.h"

namespace mitk {

//##ModelId=3ED953070392
class LevelWindowProperty : public BaseProperty
{

protected:
    //##ModelId=3ED953090113
    mitk::LevelWindow m_LevWin;

public:
    mitkClassMacro(LevelWindowProperty, BaseProperty);

    //##ModelId=3ED953090121
    LevelWindowProperty();

    //##ModelId=3ED953090122
    virtual ~LevelWindowProperty();

    //##ModelId=3ED953090124
    virtual bool operator==(const BaseProperty& property) const;

    //##ModelId=3ED953090133
    const mitk::LevelWindow & GetLevelWindow() const;

    //##ModelId=3ED953090135
    void SetLevelWindow(const mitk::LevelWindow &levWin);
};

} // namespace mitk



#endif /* MITK_LEVEL_WINDOW_PROPERTY_H_ */
