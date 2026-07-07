/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetDataInteractor_h
#define mitkPointSetDataInteractor_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkSmartPointer.h>
#include <mitkCommon.h>
#include <mitkDataInteractor.h>
#include <MitkCoreExports.h>
#include <mitkPointSet.h>

namespace mitk
{
  /**
   * \brief Implementation of the PointSetInteractor.
   *
   * Interactor that operates on a point set and supports:
   * - adding points
   * - removing points
   * - moving single points
   * - moving the complete point set
   * - selecting/unselecting a point
   *
   * in 2D and 3D render windows.
   *
   * \warning If this Interactor is assigned (SetDataNode) an empty mitk::DataNode it creates a point set.
   * Changing the point set of the assigned mitk::DataNode after this assignment will cause the
   * mitk::PointSetDataInteractor to not work properly. The usage must follow this general scheme:
   *
   * \code
   * // Set up interactor
   * m_CurrentInteractor = mitk::PointSetDataInteractor::New();
   * m_CurrentInteractor->LoadStateMachine("PointSet.xml");
   * m_CurrentInteractor->SetEventConfig("PointSetConfig.xml");
   * // Create new PointSet which will receive the interaction input
   * m_TestPointSet = mitk::PointSet::New();
   * // Add the point set to the DataNode *before* the DataNode is added to the PointSetDataInteractor
   * m_TestPointSetNode->SetData(m_TestPointSet);
   * // Finally add the DataNode (which already is added to the DataStorage) to the PointSetDataInteractor
   * m_CurrentInteractor->SetDataNode(m_TestPointSetNode);
   * \endcode
   *
   * \ingroup Interaction
   * \sa DataInteractor
   * \sa PointSet
   */
  class MITKCORE_EXPORT PointSetDataInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(PointSetDataInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set the maximum distance for selecting a point by position.
     *
     * Sets the maximum distance that is accepted when looking for a point
     * at a certain position using the GetPointIndexByPosition function.
     *
     * \param accuracy The selection accuracy (distance threshold) in world units.
     */
    void SetAccuracy(float accuracy);

    /**
     * \brief Set the maximum number of points allowed in the point set.
     *
     * Default is zero, which allows an infinite number of points.
     * When the maximum is reached, an InternalEvent "MaximalNumberOfPoints"
     * is fired.
     *
     * \param maxNumber Maximum number of allowed points (0 for unlimited).
     */
    void SetMaxPoints(unsigned int maxNumber = 0);

    /**
     * \brief Enable or disable point movement.
     * \param enabled If true (default), points can be moved interactively.
     */
    void EnableMovement(bool enabled = true);

    /**
     * \brief Enable or disable point removal.
     * \param enabled If true (default), points can be removed interactively.
     */
    void EnableRemoval(bool enabled = true);

    /**
     * \brief Enable or disable point placement in 3D render windows.
     * \param enabled If true (default), points can be placed in 3D views.
     */
    void Enable3DPointPlacement(bool enabled = true);

    /**
     * \brief Set the boundaries within which points can be placed.
     *
     * Points outside the given geometry will be rejected on add, and
     * clamped during movement.
     *
     * \param geometry The geometry defining the allowed region, or nullptr to remove bounds.
     */
    void SetBounds(BaseGeometry* geometry);

  protected:
    PointSetDataInteractor();
    ~PointSetDataInteractor() override;

    /**
     * Here actions strings from the loaded state machine pattern are mapped to functions of
     * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
     */
    void ConnectActionsAndFunctions() override;

    /**
     * This function is called when a DataNode has been set/changed.
     * It is used to initialize the DataNode, e.g. if no PointSet exists yet it is created
     * and added to the DataNode.
     */
    void DataNodeChanged() override;

    /**
     * \brief Return index in PointSet of the point that is within given accuracy to the provided position.
     *
     * Assumes that the DataNode contains a PointSet, if so it iterates over all points
     * in the DataNode to check if it contains a point near the pointer position.
     * If a point is found its index-position is returned, else -1 is returned.
     */
    virtual int GetPointIndexByPosition(Point3D position, unsigned int time = 0, float accuracy = -1);

    /**
     * \brief Check whether the pointer is over a point in the point set.
     * \return true if a point is within selection accuracy of the event position.
     */
    virtual bool CheckSelection(const InteractionEvent *interactionEvent);

    /**
     * \brief Check whether a new point can be added.
     *
     * Returns true unless 3D point placement is disabled and the event
     * originates from a 3D render window.
     */
    virtual bool CanAddPoint(const InteractionEvent *interactionEvent);

    /** Adds a point at the given coordinates.
     *  Every time a point is added it is also checked if the maximal number of points is reached,
     *  and if so an InternalEvent with the signal name "MaxNumberOfPoints" is triggered.
     */
    virtual void AddPoint(StateMachineAction *, InteractionEvent *event);

    /** Removes point that is selected */
    virtual void RemovePoint(StateMachineAction *, InteractionEvent *interactionEvent);

    /**
     * Checks if new point is close enough to an old one,
     * if so, trigger the ClosedContour signal which can be caught by the state machine.
     */
    virtual void IsClosedContour(StateMachineAction *, InteractionEvent *);

    /**
     * Moves the currently selected point to the new coordinates.
     */
    virtual void MovePoint(StateMachineAction *, InteractionEvent *);

    /**
     * Initializes the movement, stores starting position.
     */
    virtual void InitMove(StateMachineAction *, InteractionEvent *interactionEvent);

    /**
     * Is called when a movement is finished, changes back to regular color.
     */
    virtual void FinishMove(StateMachineAction *, InteractionEvent *);

    /**
     * Selects a point from the PointSet as currently active.
     */
    virtual void SelectPoint(StateMachineAction *, InteractionEvent *);

    /**
     * Unselects a point at the given coordinate.
     */
    virtual void UnSelectPointAtPosition(StateMachineAction *, InteractionEvent *);

    /**
     * Unselects all points out of reach.
     */
    virtual void UnSelectAll(StateMachineAction *, InteractionEvent *);

    /**
     * \brief Update the member variable that holds the point set.
     *
     * Re-reads the PointSet from the DataNode, evaluating the time step of the sender.
     */
    virtual void UpdatePointSet(StateMachineAction *stateMachineAction, InteractionEvent *);

    /**
     * Calls for inactivation of the DataInteractor
     */
    virtual void Abort(StateMachineAction *, InteractionEvent *);

    /** \brief Delete the currently selected point via keyboard action. */
    virtual void KeyDelete(StateMachineAction *, InteractionEvent *);

    /** \brief to calculate a direction vector from last point and actual
     * point
     */
    Point3D m_LastPoint;

    /** \brief summ-vector for Movement */
    Vector3D m_SumVec;

    // DATA
    PointSet::Pointer m_PointSet;
    int m_MaxNumberOfPoints;   // maximum of allowed number of points
    float m_SelectionAccuracy; // accuracy that's needed to select a point
    bool m_IsMovementEnabled;
    bool m_IsRemovalEnabled;
    bool m_Is3DPointPlacementEnabled;
    mitk::BaseGeometry::Pointer m_Bounds;

    // FUNCTIONS
    void UnselectAll(unsigned int timeStep, ScalarType timeInMs);
    void SelectPoint(int position, unsigned int timeStep, ScalarType timeInMS);
  };
}
#endif
