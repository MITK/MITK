#ifndef HIERARCHICALINTERACTOR_H_HEADER_INCLUDED
#define HIERARCHICALINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkInteractor.h"
#include <vector>

namespace mitk {
 
//##Documentation
//## @brief Interface for an HierarchicalInteractor.
//##
//## A hierarchicalInteractor administrates the forewarding of events to other Interactors or even HierarchicalInteractors
class HierarchicalInteractor : public Interactor
{
public:

  typedef std::vector<Interactor::Pointer> InteractorList;
  typedef std::vector<Interactor::Pointer>::iterator InteractorListIter;

  mitkClassMacro(HierarchicalInteractor,Interactor);

  //##Documentation
  //## @brief Constructor
  HierarchicalInteractor(const char * type, DataTreeNode* dataTreeNode);

  //##Documentation
  //## @brief Destructor
  ~HierarchicalInteractor(){}

  //##Documentation
  //## @brief Check the interaction mode
  bool IsSubSelected() const;

  //##Documentation
  //## @brief handles an Event accordingly to its current Modus
  virtual bool HandleEvent(StateEvent const* stateEvent, int objectEventId, int groupEventId);

  //##Documentation
  //## @brief Executes Side-Effects
  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);
  
  //##Documentation
  //## @brief Adds an Interactor to the list of administrated Interactors
  bool AddInteractor(Interactor::Pointer interactor);

protected:
  //##Documentation
  //## @brief transmitts the event
  bool TransmitEvent( StateEvent const* stateEvent, int objectEventId, int groupEventId );

  //##Documentation
  //## @brief stores pointers on hierarchical lower Interactors, that are in Mode selected or Subselected
  InteractorList m_SelectedInteractors;

  //##Documentation
  //## @brief stores all lower Interactors
  InteractorList m_AllInteractors;
};

}//namespace mitk
#endif /* HIERARCHICALINTERACTOR_H_HEADER_INCLUDED */
