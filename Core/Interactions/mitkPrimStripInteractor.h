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


#ifndef MITKPRIMSTRIPINTERACTOR_H_HEADER_INCLUDED
#define MITKPRIMSTRIPINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkHierarchicalInteractor.h>
#include "mitkLineInteractor.h"

namespace mitk
{
  class DataTreeNode;

  //##Documentation
  //## @brief Interaction with a list of many primitives.
  //##
  //## Interact with a list of interactables e.g. lines and stripes
  //## The whole list can be selected by selecting all the other lower interactables.
  //## the selection of the whole dataSet is only possible by seting the property of the DataTreeNode.
  //## @ingroup Interaction
  class MITK_CORE_EXPORT PrimStripInteractor: public HierarchicalInteractor
  {
  public:
    mitkClassMacro(PrimStripInteractor, HierarchicalInteractor);
    mitkNewMacro2Param(Self, const char*, DataTreeNode*);

    //##Documentation
    //## @brief derived from mitk::HierarchicalInteractor; calculates Jurisdiction according to lines and points
    //##
    //## standard mathod can not be used, since it doesn't differ in cells. It would calculate a boundingBox according to points.
    virtual float CalculateJurisdiction(StateEvent const* stateEvent) const;

  protected:
    //##Documentation
    //##@brief Constructor 
    PrimStripInteractor(const char * type, DataTreeNode* dataTreeNode);

    //##Documentation
    //##@brief Destructor 
    virtual ~PrimStripInteractor();

    //##Documentation
    //## @brief Executes Actions
    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

  private:
    //##Documentation
    //## @brief lower Line Interactor 
    mitk::LineInteractor::Pointer m_LineInteractor;

    //##Documentation
    //## @brief stores the current CellId this Statemachine works in
    unsigned int m_CurrentCellId;

    //##Documentation
    //## @brief stores the coordinate in the beginning of a movement. It is neede to declare undo-information
    Point3D m_MovementStart;

    //##Documentation
    //## @brief stores the coordinate of the last given MousePosition to calculate a vector
    Point3D m_OldPoint;

  };
}
#endif /* MITKPRIMSTRIPINTERACTOR_H_HEADER_INCLUDED */
