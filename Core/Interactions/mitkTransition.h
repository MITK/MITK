#ifndef TRANSITION_H_HEADER_INCLUDED_C19AE06B
#define TRANSITION_H_HEADER_INCLUDED_C19AE06B

#include "mitkCommon.h"
#include <string>


namespace mitk {
class State;

//##ModelId=3E5A399E033E
class Transition
{
  public:
    //##ModelId=3E5A3A77006F
	Transition(std::string name, int nextStateId, int eventId, int sideEffectId);

    //##ModelId=3E5A3BAE03B5
    std::string GetName() const;

    //##ModelId=3E5A3BCE00E0
	mitk::State* GetNextState() const;

    //##ModelId=3E68CB99014B
	int GetNextStateId() const;

    //##ModelId=3E5A3BF100F5
    int GetSideEffectId() const;

    //##ModelId=3E5A3C1D0289
    bool IsEvent(int eventId) const;

    //##ModelId=3E6370080067
    setNextState(State* state);

  private:
    //##ModelId=3E5A39C102B2
	std::string m_Name;

    //##ModelId=3E5A3A060366
    State* m_NextState;

    //##ModelId=3E5A3A280007
    int m_NextStateId;

    //##ModelId=3E5A3A3C0331
    int m_EventId;

    //##ModelId=3E5A3A55017F
    int m_SideEffectId;

};

} // namespace mitk



#endif /* TRANSITION_H_HEADER_INCLUDED_C19AE06B */
