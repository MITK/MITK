#ifndef MITKPRIMSTRIPINTERACTOR_H_HEADER_INCLUDED
#define MITKPRIMSTRIPINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkInteractor.h>
#include <map>

namespace mitk
{
class DataTreeNode;

//##Documentation
//## @brief Interaction with a list of many primitives.
//## @ingroup Interaction
//## Interact with a list of interactables e.g. lines and stripes
//## The whole list can be selected by selecting all the other lower interactables.
//## the selection of the whole dataSet is only possible by seting the property of the DataTreeNode.
class PrimStripInteractor: public Interactor
{
public:
  mitkClassMacro(PrimStripInteractor, Interactor);
  
  typedef int IdType;
  typedef mitk::Interactor PrimElement;
  typedef std::map<IdType, PrimElement*> PrimList;
      
  //##Documentation
  //##@brief Constructor 
  PrimStripInteractor(std::string type, DataTreeNode* dataTreeNode);

  //##Documentation
  //##@brief Destructor 
  virtual ~PrimStripInteractor();

  //##Documentation
  //##@brief returns the Id of this point
  int GetId();

protected:
  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

private:
  //##Documentation
  //## @brief store all different geometrical objects (primitives e.g. lines) with its ID's and pointer
  PrimList m_PrimList;

  //##Documentation
  //## @brief Identificationnumber of that strip
  int m_Id;

};
}
#endif /* MITKGEOMSTRIPINTERACTOR_H_HEADER_INCLUDED */
