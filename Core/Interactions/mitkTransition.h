#ifndef TRANSITION_H_HEADER_INCLUDED_C19AE06B
#define TRANSITION_H_HEADER_INCLUDED_C19AE06B

#include "mitkCommon.h"
#include "State.h"

namespace mitk {

//##ModelId=3E5A399E033E
class Transition
{
  public:
    //##ModelId=3E5A3A77006F
    Transition(string tName, int tId, int tnextStateId, int tEventId, int tSideEffectId);

    //##ModelId=3E5A3BAE03B5
    string GetName() const;

    //##ModelId=3E5A3BCE00E0
    State* GetNextState() const;

    //##ModelId=3E5A3BF100F5
    int GetSideEffectId() const;

    //##ModelId=3E5A3C1D0289
    bool IsEvent(int tEventId) const;

    //##ModelId=3E6370080067
    setNextState(State* tState) const;

  private:
    //##ModelId=3E5A39C102B2
    string name;

    //##ModelId=3E5A39E9002F
    int id;

    //##ModelId=3E5A3A060366
    State* nextState;

    //##ModelId=3E5A3A280007
    int nextStateId;

    //##ModelId=3E5A3A3C0331
    int eventId;

    //##ModelId=3E5A3A55017F
    int sideEffectId;

};

} // namespace mitk



#endif /* TRANSITION_H_HEADER_INCLUDED_C19AE06B */
