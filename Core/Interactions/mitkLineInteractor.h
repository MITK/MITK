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


#ifndef MITKLINEINTERACTOR_H_HEADER_INCLUDED
#define MITKLINEINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkHierarchicalInteractor.h>
#include "mitkPointSnapInteractor.h"
#include <mitkVector.h>

namespace mitk
{
  class DataTreeNode;
  class StateEvent;

  //##Documentation
  //## @brief Interaction with a line between two Points.
  //##
  //## Interact with a line drawn between two declared points. 
  //## The line can be selected, which selects its edges (the two points), so that the line can be moved.
  //## @ingroup Interaction
  class LineInteractor : public HierarchicalInteractor
  {
  public:
    mitkClassMacro(LineInteractor, HierarchicalInteractor);
    mitkNewMacro2Param(Self, const char*, DataTreeNode*);

    //##Documentation
    //## @brief derived from mitk::HierarchicalInteractor; calculates Jurisdiction according to lines
    //##
    //## standard mathod can not be used, since it doesn't differ in cells or lines. It would calculate a boundingBox according to points.
    virtual float CalculateJurisdiction(StateEvent const* stateEvent) const;

  protected:
    //##Documentation
    //##@brief Constructor 
    LineInteractor(const char * type, DataTreeNode* dataTreeNode);

    //##Documentation
    //##@brief Destructor 
    virtual ~LineInteractor();

    //##Documentation
    //## @brief unselect all Lines that exist in the mesh
    virtual void DeselectAllLines();

    //##Documentation
    //## @brief Executes Actions
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

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
    PointSnapInteractor::Pointer m_PointInteractor;

  };
}
#endif /* MITKLINEINTERACTOR_H_HEADER_INCLUDED */
