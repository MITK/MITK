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


#ifndef MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED
#define MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkHierarchicalInteractor.h>
#include "mitkPrimStripInteractor.h"

namespace mitk
{
  class DataTreeNode;

  //##Documentation
  //## @brief Interaction with a Polygon, a closed set of lines.
  //##
  //## Interact with a Polygon. 
  //## @ingroup Interaction
  class MITK_CORE_EXPORT PolygonInteractor : public HierarchicalInteractor
  {
  public:
    mitkClassMacro(PolygonInteractor, HierarchicalInteractor);
    mitkNewMacro2Param(Self, const char*, DataTreeNode*);

    //##Documentation
    //## @brief derived from mitk::HierarchicalInteractor; calculates Jurisdiction according to cells of a mesh
    //##
    //## standard mathod can not be used, since it doesn't differ in cells. It would calculate a boundingBox according to points.
    virtual float CalculateJurisdiction(StateEvent const* stateEvent) const;

  protected:
    //##Documentation
    //##@brief Constructor 
    PolygonInteractor(const char * type, DataTreeNode* dataTreeNode);

    //##Documentation
    //##@brief Destructor 
    virtual ~PolygonInteractor();

    //##Documentation
    //## @brief Executes Actions
    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

    //##Documentation
    //## @brief Unselect all cells of a mesh
    virtual void DeselectAllCells();

  private:

    //##Documentation
    //## @brief lower Line Interactor 
    mitk::PrimStripInteractor::Pointer m_PrimStripInteractor;

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
#endif /* MITKPOLYGONINTERACTOR_H_HEADER_INCLUDED */
