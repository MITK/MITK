#ifndef STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD
#define STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD

#include "mitkCommon.h"
#include "State.h"
#include <qxml.h>

namespace mitk {

//##ModelId=3E5A39550068
//##Documentation
//## a xml file including the states and the transitions is read.
//## according to the structur all States are generated.
//## Then all transitions are generated and the pointers are set
//## (Transition::setNextState(State* tState))
class StateMachineFactory
{
  public:
    //##ModelId=3E5B4144024F
	  static State* GetStartState(std::string type);

    //##ModelId=3E5B41730261
	  static bool LoadBehavior(std::string fileName);

  private:
    //##ModelId=3E5B428F010B
    static bool ConnectStates(vector<State*> states);

    //##ModelId=3E5B423003DF
	map<std::string,State*> startStates;

};

} // namespace mitk



#endif /* STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD */
