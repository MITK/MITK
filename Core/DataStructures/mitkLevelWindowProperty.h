#ifndef MITK_LEVEL_WINDOW_PROPERTY_H_
#define MITK_LEVEL_WINDOW_PROPERTY_H_

#include "mitkCommon.h"
#include "BaseProperty.h"
#include "LevelWindow.h"

namespace mitk {


//##ModelId=3E86A20C00FD
class LevelWindowProperty : public BaseProperty
{

protected:
    //##ModelId=3E86A3B00061
	mitk::LevelWindow m_LevWin;

public:
    mitkClassMacro(LevelWindowProperty, BaseProperty);


    //##ModelId=3E86A3450130
    LevelWindowProperty();

    //##ModelId=3E86A345014E
    virtual ~LevelWindowProperty();

    //##ModelId=3E86A35F000B
    virtual bool operator==(const BaseProperty& property) const;

    //##ModelId=3E86AABB0371
		mitk::LevelWindow & GetLevelWindow();

		//##ModelId=3E86AABB0399
		void SetLevelWindow(mitk::LevelWindow &levWin);
};

} // namespace mitk



#endif /* MITK_LEVEL_WINDOW_PROPERTY_H_ */
