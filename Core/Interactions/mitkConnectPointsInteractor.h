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


#ifndef MITKCONNECTPOINTSINTERACTOR_H_HEADER_INCLUDED_C11202FF
#define MITKCONNECTPOINTSINTERACTOR_H_HEADER_INCLUDED_C11202FF

#include "mitkCommon.h"
#include "mitkVector.h"
#include <mitkInteractor.h>

namespace mitk
{
  class DataTreeNode;
  //##Documentation
  //## @brief Interaction for mitk::Mesh: Connect existing points to lines
  //## @ingroup Interaction
  class ConnectPointsInteractor : public Interactor
  {
  public:
    mitkClassMacro(ConnectPointsInteractor, Interactor);
    mitkNewMacro3Param(Self, const char*, DataTreeNode*, int);
    mitkNewMacro2Param(Self, const char*, DataTreeNode*);

    //##Documentation
    //## @brief Sets the amount of precision
    void SetPrecision( unsigned int precision );

    //##Documentation
    //## @brief calculates how good the data, this statemachine handles, is hit by the event.
    //## 
    //## overwritten, cause we don't look at the boundingbox, we look at each point
    virtual float CalculateJurisdiction(StateEvent const* stateEvent) const;


  protected:
    //##Documentation
    //##@brief Constructor with Param n for limited Set of Points
    //##
    //## if no n is set, then the number of points is unlimited*
    ConnectPointsInteractor(const char * type, DataTreeNode* dataTreeNode, int n = -1);

    virtual ~ConnectPointsInteractor();

    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

    //##ModelId=3F017B320121
    //##Documentation
    //## @brief deselects the Points in the PointSet.
    //## supports Undo if enabled
    void UnselectAll();

    //##ModelId=3F05C0700185
    //##Documentation
    //## @brief Selects the point.
    //## supports Undo if enabled.
    //## @param position is the index of the point that has to be selected
    void SelectPoint( int position );

  private:

    //##ModelId=3F017B3200E3
    //##Documentation
    //## @brief the number of possible points in this object
    //##
    //## if -1, then no limit set
    int m_N;

    //##Documentation
    //## @brief stores the current CellId this Statemachine works in
    unsigned int m_CurrentCellId;

    //##ModelId=3F0AF6CF00C2
    //##Documentation
    //## @brief to calculate a direction vector from last point and actual point
    Point3D m_LastPoint;

    //##Documentation
    //## @brief summ-vector for Movement
    Vector3D m_SumVec;

    //##Documentation
    //## @brief to store the value of precision to pick a point
    unsigned int m_Precision;
  };
}
#endif /* MITKCONNECTPOINTSINTERACTOR_H_HEADER_INCLUDED_C11202FF */
