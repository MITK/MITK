/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF
#define MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF

#include "mitkCommon.h"
#include "mitkVector.h"
#include <mitkInteractor.h>

namespace mitk
{
  class DataTreeNode;
  //##ModelId=3F017AFD00C3
  //##Documentation
  //## @brief Interaction with a set of points.
  //##
  //## Points can be added, removed and moved.
  //## @ingroup Interaction
  class PointSetInteractor : public Interactor
  {
  public:
    //##ModelId=3F017B3200F2
    mitkClassMacro(PointSetInteractor, Interactor);

    //##ModelId=3F017B3200F5
    //##Documentation
    //##@brief Constructor with Param n for limited Set of Points
    //##
    //## if no n is set, then the number of points is unlimited*
    PointSetInteractor(const char * type, DataTreeNode* dataTreeNode, int n = -1);

    //##ModelId=3F017B320103
    virtual ~PointSetInteractor();

    //##Documentation
    //## @brief clears all the elements in the list with undo-functionality
    //##
    //## moved due to use in functionalities
    void Clear();

    //##Documentation
    //## @brief Sets the amount of precision
    void SetPrecision( unsigned int precision );

    //##Documentation
    //## @brief calculates how good the data, this statemachine handles, is hit by the event.
    //## 
    //## overwritten, cause we don't look at the boundingbox, we look at each point
    virtual float CalculateJurisdiction(StateEvent const* stateEvent) const;


  protected:
    //##ModelId=3F017B320105
    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

    //##ModelId=3F017B320121
    //##Documentation
    //## @brief Deselects the Points in the PointSet.
    //## supports Undo if enabled
    void UnselectAll();

    //##ModelId=3F05C0700185
    //##Documentation
    //## @brief Selects the point.
    //## supports Undo if enabled.
    //## @param position Needed for declaring operations
    void SelectPoint( int position );

  private:

    //##ModelId=3F017B3200E3
    //##Documentation
    //## @brief the number of possible points in this object
    //##
    //## if -1, then no limit set
    int m_N;

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
#endif /* MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF */
