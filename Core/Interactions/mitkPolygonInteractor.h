#ifndef MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED
#define MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkInteractor.h>
#include "mitkPrimStripInteractor.h"
#include <map>

namespace mitk
{
class DataTreeNode;

//##Documentation
//## @brief Interaction with a Polygon, a closed set of lines.
//## @ingroup Interaction
//## Interact with a Polygon. Basicly it only keeps the list of elements closed (closes a PrimSrip)
class PolygonInteractor : public Interactor
{
public:
  mitkClassMacro(PolygonInteractor, Interactor);
  
  typedef int IdType;
  typedef mitk::Interactor PrimElement;
  typedef std::map<IdType, PrimElement*> PrimList;
      
  //##Documentation
  //##@brief Constructor 
  PolygonInteractor(std::string type, DataTreeNode* dataTreeNode);

  //##Documentation
  //##@brief Destructor 
  virtual ~PolygonInteractor();

  //##Documentation
  //##@brief returns the Id of this point
  int GetId();

protected:
  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

private:
  //##Documentation
  //## @brief lower Interactor for the rest of the interaction
  PrimStripInteractor *m_PrimStrip;

  //##Documentation
  //## @brief Identificationnumber of that Polygon
  int m_Id;

};
}
#endif /* MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED */
