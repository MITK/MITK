#ifndef MiniAppManager_h
#define MiniAppManager_h

#include <mitkCommon.h>

struct MiniAppManager
{

public:

    typedef int (*MiniAppFunction)(int argc, char* argv[]);

public:

    static MiniAppManager* GetInstance();

    // Attention: Name of the miniApp must be the last argument!!!
    // it will be cut off from the rest of the arguments and then
    // the app will be run
    int RunMiniApp(int argc, char* argv[]);

    //
    // Add miniApp
    //
    MiniAppFunction AddFunction(const std::string& name, MiniAppFunction func);

protected:

    std::map< std::string, MiniAppFunction >  m_Functions;
};

//
// Register miniApps
//
#define RegisterDiffusionMiniApp(functionName) \
    static MiniAppManager::MiniAppFunction MiniApp##functionName = \
    MiniAppManager::GetInstance()->AddFunction(#functionName, &functionName)
#endif
