#ifndef STATE_H_HEADER_INCLUDED_C19A8A5D
#define STATE_H_HEADER_INCLUDED_C19A8A5D

#include "mitkCommon.h"
#include "Transition.h"

namespace mitk {

//##ModelId=3E5A3986027B
class State
{
  public:
    //##ModelId=3E5B2A9203BD
    State(string stateName, int stateId);

    //##ModelId=3E5B2B2E0304
    bool AddTransition(string transitionName, int transitionId, int nextStateId, int eventId, int sideEffectId) const;

    //##ModelId=3E5B2B9000AC
    //##Documentation
    //## hashmap-lookup and returning the Transition. if not located, then NULL
    Transition* GetTransition(int eventId) const;

    //##ModelId=3E5B2C0503D5
    string GetName() const;

    //##ModelId=3E5B2C14016A
    int GetId() const;

    //##ModelId=3E64B4360017
    //##Documentation
    //## locates an eventId in the hashmap and returns true if located, else
    //## false
    bool locateTransition(int tEventId) const;

  private:
    //##ModelId=3E5B2A220069
    string name;

    //##ModelId=3E5B2A350338
    int id;

    //##ModelId=3E5B2A460057
    hashMap<Transition> transitions;

};

} // namespace mitk



#endif /* STATE_H_HEADER_INCLUDED_C19A8A5D */
