#ifndef STATE_H_HEADER_INCLUDED_C19A8A5D
#define STATE_H_HEADER_INCLUDED_C19A8A5D

// #include "mitkCommon.h"
#include "Transition.h"
#include <string>
#include <map>


//##ModelId=3E6907B400B4
typedef std::map<int,mitk::State*> StateMap;
//##ModelId=3E6907B400C4
typedef std::map<int,mitk::Transition> TransitionMap;

//##ModelId=3E72243D0263
typedef std::map<int,mitk::State *>::iterator StateMapIter;
//##ModelId=3E72243D0273
typedef std::map<int,mitk::Transition>::iterator TransMapIter;


namespace mitk {

//##ModelId=3E5A3986027B
class State
{
  public:
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

    //##ModelId=3E64B4360017
    //##Documentation
    //## locates an eventId in the hashmap and returns true if located, else
    //## false
    bool LocateTransition(int eventId) const;

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
