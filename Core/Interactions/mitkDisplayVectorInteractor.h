#ifndef MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB
#define MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB

#include "mitkCommon.h"
#include "BaseRenderer.h"
#include "Roi.h"

namespace mitk {

    class Operation;
    class OperationActor;

//##ModelId=3EF221E9001C
class DisplayVectorInteractor : public Roi
{
  public:
    //##ModelId=3EF222410127
    virtual void ExecuteOperation(Operation* operation);
    //##ModelId=3EF2229F00F0
    DisplayVectorInteractor(std::string type, mitk::OperationActor* destination=NULL);

    //##ModelId=3EF2229F010E
    virtual ~DisplayVectorInteractor();

  protected:
    //##ModelId=3EF2224401CB
    virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int groupEventId, int objectEventId);

  private:
    //##ModelId=3EF1D2A002DD
    BaseRenderer::Pointer m_Sender;

    //##ModelId=3EF1D2A00341
    mitk::Point2D m_StartDisplayCoordinate;
    mitk::Point2D m_LastDisplayCoordinate;
    mitk::Point2D m_CurrentDisplayCoordinate;

    //##ModelId=3EF231670106
    OperationActor* m_Destination;
};

} // namespace mitk



#endif /* MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB */

