#ifndef MITKPOINTINTERACTOR_H_HEADER_INCLUDED
#define MITKPOINTINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkInteractor.h>
#include <mitkOperationActor.h>
#include <mitkVector.h>

namespace mitk
{
class DataTreeNode;

//##Documentation
//## @brief Interaction with a point
//## @ingroup Interaction
//## Interact with a point: set point, select point, move point and remove point
//## All Set-operations are done through the method "ExecuteAction".
//## the identificationnumber of this point is set by this points and evalued from an empty place in the DataStructure
class PointInteractor : public Interactor
{
public:
  mitkClassMacro(PointInteractor, Interactor);

  //##Documentation
  //##@brief Constructor 
  PointInteractor(const char * type, DataTreeNode* dataTreeNode, int id);

  //##Documentation
  //##@brief Destructor 
  virtual ~PointInteractor();

  //##Documentation
  //##@brief returns the Id of this point
  int GetId();

protected:
  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

private:
  //##Documentation
  //## @brief stores the ID of the point
  int m_Id;

  //##Documentation
  //## @brief to calculate a direction vector from last point and actual point
  Point3D m_LastPoint;

};
}
#endif /* MITKPOINTINTERACTOR_H_HEADER_INCLUDED */
