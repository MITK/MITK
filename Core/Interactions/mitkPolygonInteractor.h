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

  //##Documentation
  //## @brief derived from mitk::HierarchicalInteractor; calculates Jurisdiction according to cells of a mesh
  //##
  //## standard mathod can not be used, since it doesn't differ in cells. It would calculate a boundingBox according to points.
  virtual float CalculateJurisdiction(StateEvent const* stateEvent) const;

protected:
  //##Documentation
  //## @brief Executes Actions
  virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );
  
  //##Documentation
  //## @brief Unselect all cells of a mesh
  virtual void DeselectAllCells();



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
