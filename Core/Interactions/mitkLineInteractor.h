#ifndef MITKLINEINTERACTOR_H_HEADER_INCLUDED
#define MITKLINEINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkHierarchicalInteractor.h>
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
class LineInteractor : public HierarchicalInteractor
{
public:
  mitkClassMacro(LineInteractor, HierarchicalInteractor);
    
  //##Documentation
  //##@brief Constructor 
  LineInteractor(const char * type, DataTreeNode* dataTreeNode);

  //##Documentation
  //##@brief Destructor 
  virtual ~LineInteractor();

protected:
  //##Documentation
  //## @brief unselect all Lines that exist in the mesh
  virtual void DeselectAllLines(int objectEventId, int groupEventId);

  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

private:
  //##Documentation
  //## to make possible the undo of a movement of a line
  Point3D m_LastPoint, m_TempPoint;

  //##Documentation
  //## to remember a lineId
  unsigned long m_CurrentLineId;
  
  //##Documentation
  //## to remember a cellId
  unsigned long m_CurrentCellId;

  //##Documentation
  //## @brief lower Point Interactor 
  PointInteractor::Pointer m_PointInteractor;

};
}
#endif /* MITKLINEINTERACTOR_H_HEADER_INCLUDED */
