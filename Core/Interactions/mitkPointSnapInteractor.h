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


#ifndef MITKPOINTSNAPINTERACTOR_H_HEADER_INCLUDED
#define MITKPOINTSNAPINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkHierarchicalInteractor.h>
#include "mitkPointInteractor.h"

namespace mitk
{
  //class DataTreeNode;

  //##Documentation
  //## @brief adds Snap to Point Interaction to PointInteractor if hierarchicaly ordered
  //##
  //## This class adds snapping to a point to the existing interaction. Hierarchicaly set 
  //## above the Pointinteractor, it checks if an already existing point lies near enough 
  //## to the MousePosition, when a Point is to be added.
  //## If so, then it only activates the point, so that the LineInteractor (hierarchicaly 
  //## above this PointSnapInteractor) can add a line to that Point.
  //## @ingroup Interaction
  class MITK_CORE_EXPORT PointSnapInteractor : public HierarchicalInteractor
  {
  public:
    mitkClassMacro(PointSnapInteractor, HierarchicalInteractor);
    mitkNewMacro2Param(Self, const char*, DataTreeNode*);

    //##Documentation
    //## @brief uses CalculateJurisdiction from PointInteractor
    virtual float CalculateJurisdiction(StateEvent const* stateEvent) const;
    //{return this->CalculateLowerJurisdiction(stateEvent);};

  protected:
    //##Documentation
    //##@brief Constructor 
    PointSnapInteractor(const char * type, DataTreeNode* dataTreeNode);

    //##Documentation
    //##@brief Destructor 
    virtual ~PointSnapInteractor();

    //##Documentation
    //## @brief select the point on the given position
    virtual void SelectPoint(int position);

    //##Documentation
    //## @brief unselect all points that exist in mesh
    virtual void DeselectAllPoints();

    //##Documentation
    //## @brief Executes Actions
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  private:
    PointInteractor::Pointer m_PointInteractor;

    unsigned int m_Position;
  };
}//namespace mitk
#endif /* MITKPOINTSNAPINTERACTOR_H_HEADER_INCLUDED */


