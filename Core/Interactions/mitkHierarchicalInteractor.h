#ifndef HIERARCHICALINTERACTOR_H_HEADER_INCLUDED
#define HIERARCHICALINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkInteractor.h"
#include <vector>

namespace mitk {
 
//##Documentation
//## @brief Interface for an HierarchicalInteractor.
//## @ingroup Interaction
//##
//## A hierarchicalInteractor administrates the forewarding of events to other Interactors or even HierarchicalInteractors
class HierarchicalInteractor : public Interactor
{
public:

  typedef std::vector<Interactor::Pointer> InteractorList;
  typedef std::vector<Interactor::Pointer>::iterator InteractorListIter;
  typedef std::vector<Interactor::Pointer>::const_iterator InteractorListConstIter;

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
  virtual bool HandleEvent( StateEvent const* stateEvent );

  //##Documentation
  //## @brief Executes Actions
  virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );
  
  //##Documentation
  //## @brief Adds an Interactor to the list of administrated Interactors
  bool AddInteractor(Interactor::Pointer interactor);

  //##Documentation
  //## @brief derived from mitk::Interactor; merges all jurisdictions of lower interactors with this jurisdiction
  virtual float CalculateJurisdiction(StateEvent const* stateEvent) const;


protected:
  //##Documentation
  //## @brief transmitts the event
  bool TransmitEvent( StateEvent const* stateEvent );

  //##Documentation
  //## @brief calculates and merges all jurisdictions of lower interactors
  float CalculateLowerJurisdiction(StateEvent const* stateEvent) const;

  //##Documentation
  //## @brief stores pointers on hierarchical lower Interactors, that are in Mode selected or Subselected
  InteractorList m_SelectedInteractors;

  //##Documentation
  //## @brief stores all lower Interactors
  InteractorList m_AllInteractors;
};

}//namespace mitk
#endif /* HIERARCHICALINTERACTOR_H_HEADER_INCLUDED */
