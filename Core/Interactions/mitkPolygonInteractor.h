#ifndef MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED
#define MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkHierarchicalInteractor.h>
#include "mitkLineInteractor.h"
#include "mitkPointSetInteractor.h"
#include <mitkVector.h>
#include <map>

namespace mitk
{
class DataTreeNode;

//##Documentation
//## @brief Interaction with a Polygon, a closed set of lines.
//## @ingroup Interaction
//## Interact with a Polygon. 
class PolygonInteractor : public HierarchicalInteractor
{
public:
  mitkClassMacro(PolygonInteractor, HierarchicalInteractor);
  
  //##Documentation
  //##@brief Constructor 
  PolygonInteractor(const char * type, DataTreeNode* dataTreeNode);

  //##Documentation
  //##@brief Destructor 
  virtual ~PolygonInteractor();

protected:
  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);
  
  //##Documentation
  //## @brief Unselect all cells of a mesh
  virtual void DeselectAllCells(int objectEventId, int groupEventId);

private:

  //##Documentation
  //## @brief lower Line Interactor 
  mitk::LineInteractor::Pointer m_LineInteractor;

  //##Documentation
  //## @brief stores the current CellId this Statemachine works in
  unsigned int m_CurrentCellId;

  //##Documentation
  //## @brief stores the position of the last added point to add the line afterwards
  //int m_PointIndex;

  //##Documentation
  //## @brief stores the coordinate in the beginning of a movement. It is neede to declare undo-information
  Point3D m_MovementStart;

  //##Documentation
  //## @brief stores the coordinate of the last given MousePosition to calculate a vector
  Point3D m_OldPoint;



};
}
#endif /* MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED */
