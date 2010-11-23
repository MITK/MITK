#ifndef MITK_WIIMOTEINTERACTOR_H
#define MITK_WIIMOTEINTERACTOR_H

// mitk
#include <mitkInteractor.h>
#include <mitkDataNode.h>
#include <mitkAction.h>
#include <mitkStateEvent.h>

// export macro
#include <mitkWiiMoteExports.h>

namespace mitk
{

class mitkWiiMote_EXPORT WiiMoteInteractor: public mitk::Interactor
{

public:

// SmartPointer macros
mitkClassMacro(WiiMoteInteractor, Interactor);
mitkNewMacro2Param(Self, const char*, DataNode*);

protected:

// SmartPointer conventions
WiiMoteInteractor(const char* type, DataNode* dataNode);
virtual ~WiiMoteInteractor();

// allow movement
virtual bool OnWiiMoteActivateButton(Action* action, const mitk::StateEvent* event);

// movement
virtual bool OnWiiMoteInput(Action* action, const mitk::StateEvent* event);

// stopping movement
virtual bool OnWiiMoteReleaseButton(Action* action, const mitk::StateEvent* event);

private:

  int m_Test;

};

}
#endif // MITK_WIIMOTEINTERACTOR_H