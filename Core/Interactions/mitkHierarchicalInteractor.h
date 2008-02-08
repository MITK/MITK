/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
  //## @ingroup Interaction
  class MITK_CORE_EXPORT HierarchicalInteractor : public Interactor
  {
  public:

    typedef std::vector<Interactor::Pointer> InteractorList;
    typedef std::vector<Interactor::Pointer>::iterator InteractorListIter;
    typedef std::vector<Interactor::Pointer>::const_iterator InteractorListConstIter;

    mitkClassMacro(HierarchicalInteractor,Interactor);
    mitkNewMacro2Param(Self, const char*, DataTreeNode*);

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
    //## @brief Constructor
    HierarchicalInteractor(const char * type, DataTreeNode* dataTreeNode);

    //##Documentation
    //## @brief Destructor
    ~HierarchicalInteractor(){}

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

    virtual void SetDataTreeNode( DataTreeNode* dataTreeNode );
  };

}//namespace mitk
#endif /* HIERARCHICALINTERACTOR_H_HEADER_INCLUDED */
