/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF
#define MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF

#include <MitkCoreExports.h>
#include "mitkNumericTypes.h"
#include <mitkInteractor.h>

namespace mitk
{
  class DataNode;

  /**
   * \brief Interaction with a set of points.
   *
   * Points can be added, removed and moved.
   * In case the interaction shall be done on a
   * loaded set of points, the associated data
   * object (mitkPointSet) needs to be loaded
   * prior to the instanciation of this object.
   * The number of points are checked and the internal
   * statemachine set to the apropriate state.
   * The management of 0 points is not supported.
   * In this case, the amount of managed points is set to 1.
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

    itkGetMacro( Precision, unsigned int );
    itkSetMacro( Precision, unsigned int );

    /**
     * \brief calculates how good the data, this statemachine handles, is hit
     * by the event.
     *
     * overwritten, cause we don't look at the boundingbox, we look at each point
     */
    virtual float CanHandleEvent(StateEvent const* stateEvent) const;

    /**
    *@brief If data changed then initialize according to numbers of loaded points
    **/
    virtual void DataChanged();

  protected:
    /**
     * \brief Constructor with Param n for limited Set of Points
     *
     * If no n is set, then the number of points is unlimited
     * n=0 is not supported. In this case, n is set to 1.
     */
    PointSetInteractor(const char * type, DataNode* dataNode, int n = -1);

    /**
     * \brief Default Destructor
     **/
    virtual ~PointSetInteractor();

    /**
    * @brief Convert the given Actions to Operations and send to data and UndoController
    */
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

    /** \brief to store the value of precision to pick a point */
    unsigned int m_Precision;
  private:

    /**
     * \brief the number of possible points in this object
     *
     * if -1, then no limit set
     */
    int m_N;

    /**
    * @brief Init the StatateMachine according to the current number of points in case of a loaded pointset.
    **/
    void InitAccordingToNumberOfPoints();
  };
}
#endif /* MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF */
