#ifndef MITKLINEINTERACTOR_H_HEADER_INCLUDED
#define MITKLINEINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkInteractor.h>
#include "mitkPointInteractor.h"
#include <mitkVector.h>

namespace mitk
{
class DataTreeNode;

//##Documentation
//## @brief Interaction with a line between two Points.
//## @ingroup Interaction
//## Interact with a line drawn between two declared points. 
//## The line can be selected, which selects its edges (the two points), so that the line can be moved.
class LineInteractor : public Interactor
{
public:
  mitkClassMacro(LineInteractor, Interactor);
    
  //##Documentation
  //##@brief Constructor 
  LineInteractor(const char * type, DataTreeNode* dataTreeNode, int cellId, int pIdA, int pIdB);

  //##Documentation
  //##@brief Destructor 
  virtual ~LineInteractor();

  //##Documentation
  //##@brief returns the Id of this point
  int GetId();

  //##Documentation
  //##@brief returns the Id of the Cell the line is inserted to
  int GetCellId();

  //##Documentation
  //##@brief returns the Id of the first point
  int GetPointIdA();
  
  //##Documentation
  //##@brief returns the Id of the second point
  int GetPointIdB();

protected:
  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

private:
  //##Documentation
  //## @brief the cell Id, the line is to be inserted to
  int m_CellId;

  //##Documentation
  //## @brief stores the ID of the first point
  int m_PIdA;

  //##Documentation
  //## @brief stores the ID of the second point
  int m_PIdB;

  //##Documentation
  //## @brief stores the ID of the line
  int m_Id;

  //##Documentation
  //## @brief the first point
  PointInteractor::Pointer m_PointA;

  //##Documentation
  //## @brief the second point
  PointInteractor::Pointer m_PointB;

  //##Documentation
  //## to make possible the undo of a movement of a line
  Point3D m_LastPoint;

};
}
#endif /* MITKLINEINTERACTOR_H_HEADER_INCLUDED */
