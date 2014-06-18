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

#ifndef mitkPointSetDataInteractor_h_
#define mitkPointSetDataInteractor_h_

#include "itkObject.h"
#include "itkSmartPointer.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkCoreExports.h>
#include "mitkDataInteractor.h"
#include <mitkPointSet.h>

namespace mitk
{
  /**
   * Class PointSetDataInteractor
   * \brief Implementation of the PointSetInteractor
   *
   * Interactor operates on a point set and supports to:
   * - add points
   * - remove points
   * - move single points
   * - move complete pointset
   * - select/unselect a point
   *
   * in 2d and 3d render windows.
   *
   * \warn If this Interactor is assigned (SetDataNode) an empty mitk::DataNode it creates a point set,
   * changing the point set of the assigned mitk::DataNode after this assignment will cause the mitk::PointSetDataInteractor
   * to not work properly. So the usage has follow this general scheme:
   *
   * \code
    // Set up interactor
    m_CurrentInteractor = mitk::PointSetDataInteractor::New();
    m_CurrentInteractor->LoadStateMachine("PointSet.xml");
    m_CurrentInteractor->SetEventConfig("PointSetConfig.xml");
    //Create new PointSet which will receive the interaction input
    m_TestPointSet = mitk::PointSet::New();
    // Add the point set to the mitk::DataNode *before* the DataNode is added to the mitk::PointSetDataInteractor
    m_TestPointSetNode->SetData(m_TestPointSet);
    // finally add the mitk::DataNode (which already is added to the mitk::DataStorage) to the mitk::PointSetDataInteractor
    m_CurrentInteractor->SetDataNode(m_TestPointSetNode);

\endcode
   *
   *
   */

  // Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
  class MITK_CORE_EXPORT PointSetDataInteractor: public DataInteractor
  {

  public:
    mitkClassMacro(PointSetDataInteractor, DataInteractor)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * Sets the maximum distance that is accepted when looking for a point at a certain position using the GetPointIndexByPosition function.
     */
    void SetAccuracy(float accuracy);


    /**
     * @brief SetMaxPoints Sets the maximal number of points for the pointset
     * Default ist zero, which result in infinite number of allowed points
     * @param maxNumber
     */
    void SetMaxPoints(unsigned int maxNumber = 0);

  protected:
    PointSetDataInteractor();
    virtual ~PointSetDataInteractor();

    /**
     * Here actions strings from the loaded state machine pattern are mapped to functions of
     * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
     */
    virtual void ConnectActionsAndFunctions();

    /**
     * This function is called when a DataNode has been set/changed.
     * It is used to initialize the DataNode, e.g. if no PointSet exists yet it is created
     * and added to the DataNode.
     */
    virtual void DataNodeChanged();

    /**
     * \brief Return index in PointSet of the point that is within given accuracy to the provided position.
     *
     * Assumes that the DataNode contains a PointSet, if so it iterates over all points
     * in the DataNode to check if it contains a point near the pointer position.
     * If a point is found its index-position is returned, else -1 is returned.
     */
    virtual int GetPointIndexByPosition(Point3D position, int time = 0, float accuracy = -1);



    virtual bool CheckSelection( const InteractionEvent* interactionEvent );

    /** Adds a point at the given coordinates.
     *  Every time a point is added it is also checked if the maximal number of points is reached,
     *  and if so an InternalEvent with the signal name "MaxNumberOfPoints" is triggered.
     */
    virtual bool AddPoint(StateMachineAction*, InteractionEvent* event);

    /** Removes point that is selected */
    virtual bool RemovePoint(StateMachineAction*, InteractionEvent*interactionEvent);

    /**
     * Checks if new point is close enough to an old one,
     * if so, trigger the ClosedContour signal which can be caught by the state machine.
     */
    virtual bool IsClosedContour(StateMachineAction*, InteractionEvent*);

    /**
     * Moves the currently selected point to the new coodinates.
     */
    virtual bool MovePoint(StateMachineAction*, InteractionEvent*);

    /**
     * Initializes the movement, stores starting position.
     */
    virtual bool InitMove(StateMachineAction*, InteractionEvent*interactionEvent);

    /**
     * Is called when a movement is finished, changes back to regular color.
     */
    virtual bool FinishMove(StateMachineAction*, InteractionEvent*);

    /**
     * Selects a point from the PointSet as currently active.
     */
    virtual bool SelectPoint(StateMachineAction*, InteractionEvent*);

    /**
     * Unselects a point at the given coordinate.
     */
    virtual bool UnSelectPointAtPosition(StateMachineAction*, InteractionEvent*);

    /**
     * Unselects all points out of reach.
     */
    virtual bool UnSelectAll(StateMachineAction*, InteractionEvent*);


    /**
     * @brief UpdatePointSet Updates the member variable that holds the point set, evaluating the time step of the sender.
     */

    virtual bool UpdatePointSet(StateMachineAction* stateMachineAction, InteractionEvent*);

    /**
     * Calls for inactivation of the DataInteractor
     */
    virtual bool Abort(StateMachineAction*, InteractionEvent*);

    /** \brief to calculate a direction vector from last point and actual
     * point
     */
    Point3D m_LastPoint;

    /** \brief summ-vector for Movement */
    Vector3D m_SumVec;

    // DATA
    PointSet::Pointer m_PointSet;
    int m_MaxNumberOfPoints; // maximum of allowed number of points
    float m_SelectionAccuracy; // accuracy that's needed to select a point

    // FUNCTIONS
    void UnselectAll(unsigned int timeStep , ScalarType timeInMs);
    void SelectPoint(int position, unsigned int timeStep  , ScalarType timeInMS);
  };

}
#endif
