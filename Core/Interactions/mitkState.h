#ifndef STATE_H_HEADER_INCLUDED_C19A8A5D
#define STATE_H_HEADER_INCLUDED_C19A8A5D

#include "mitkCommon.h"
#include "Transition.h"
#include <string>
#include <map>
#include <set>


namespace mitk {

//##ModelId=3E5A3986027B
//##Documentation
//## @brief represents one state with all its necessary information
//## @ingroup Interaction
//## Name and ID are stored. Also methods for building up, connecting and
//## parsing for well formed statemachines are present.
//## This class holds a map of transitions to next States. 
class State
{
  public:
	//##ModelId=3F0177080382
	typedef std::map<int,mitk::State*> StateMap;
	//##ModelId=3F0177080393
	typedef std::map<int,mitk::Transition> TransitionMap;

	//##ModelId=3F01770803B1
	typedef std::map<int,mitk::State *>::iterator StateMapIter;
	//##ModelId=3F01770803C0
	typedef std::map<int,mitk::Transition>::iterator TransMapIter;

	//##ModelId=3F01770803D0
	typedef std::map<int,mitk::Transition>::const_iterator TransMapConstIter;
    
	//##ModelId=3E5B2A9203BD
	State(std::string name, int id);

    //##ModelId=3E5B2B2E0304
	bool AddTransition(std::string transitionName, int nextStateId, int eventId, int sideEffectId);

    //##ModelId=3E5B2B9000AC
    //##Documentation
    //## hashmap-lookup and returning the Transition. if not located, then NULL
    const Transition* GetTransition(int eventId) const;

    //##ModelId=3E5B2C0503D5
    std::string GetName() const;

    //##ModelId=3E5B2C14016A
    int GetId() const;

    //##ModelId=3E7757280208
	//##Documentation
    //## gives all next States back. To parse through all States.
	std::set<int> GetAllNextStates() const;

    //##ModelId=3E64B4360017
    //##Documentation
	//## to check, if this Event has a Transition. 
	//## for menu Behavior e.g.
    bool IsValidEvent(int eventId) const;

    //##ModelId=3E68C573013F
	//##Documentation
    //## searches dedicated States of all Transitions and
	//## sets *nextState of these Transitions.
	//## allStates is a List of all build States of that StateMachine
	bool ConnectTransitions(StateMap *allStates);


  private:
    //##ModelId=3E5B2A220069
	std::string m_Name;

    //##ModelId=3E5B2A350338
    int m_Id;

    //##ModelId=3E5B2A460057
	TransitionMap m_Transitions;

};

} // namespace mitk



#endif /* STATE_H_HEADER_INCLUDED_C19A8A5D */
