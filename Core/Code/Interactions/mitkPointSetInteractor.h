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


#ifndef MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF
#define MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF

#include "mitkCommon.h"
#include "mitkVector.h"
#include <mitkInteractor.h>

namespace mitk
{
  class DataNode;

  /**
   * \brief Interaction with a set of points.
   *
   * Points can be added, removed and moved.
   * \ingroup Interaction
   */
  class MITK_CORE_EXPORT PointSetInteractor : public Interactor
  {
  public:
    mitkClassMacro(PointSetInteractor, Interactor);
    mitkNewMacro3Param(Self, const char*, DataNode*, int);
    mitkNewMacro2Param(Self, const char*, DataNode*);

    /**
     * @brief Clears all the elements from the given timeStep in the list with undo-functionality and 
     * resets the statemachine
     */
    void Clear( unsigned int timeStep = 0, ScalarType timeInMS = 0.0 );

    /** \brief Sets the amount of precision */
    void SetPrecision( unsigned int precision );

    /**
     * \brief calculates how good the data, this statemachine handles, is hit
     * by the event.
     *
     * overwritten, cause we don't look at the boundingbox, we look at each point
     */
    virtual float CanHandleEvent(StateEvent const* stateEvent) const;


  protected:
    /**
     * \brief Constructor with Param n for limited Set of Points
     *
     * if no n is set, then the number of points is unlimited*
     */
    PointSetInteractor(const char * type, DataNode* dataTreeNode, int n = -1);

    /**
     * \brief Default Destructor
     **/
    virtual ~PointSetInteractor();

    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

    /** \brief Deselects the Points in the PointSet.
     * supports Undo if enabled
     */
    void UnselectAll( unsigned int timeStep = 0, ScalarType timeInMS = 0.0 );

    /**
     * \brief Selects the point.
     * supports Undo if enabled.
     * \param position Needed for declaring operations
     */
    void SelectPoint( int position, unsigned int timeStep = 0, ScalarType timeInMS = 0.0 );

    /** \brief to calculate a direction vector from last point and actual
     * point 
     */
    Point3D m_LastPoint;

    /** \brief summ-vector for Movement */
    Vector3D m_SumVec;

  private:

    /**
     * \brief the number of possible points in this object
     *
     * if -1, then no limit set
     */
    int m_N;

    /** \brief to store the value of precision to pick a point */
    unsigned int m_Precision;
  };
}
#endif /* MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF */
