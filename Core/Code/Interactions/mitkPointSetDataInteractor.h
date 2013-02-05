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
#include <MitkExports.h>
#include "mitkDataInteractor.h"
#include <mitkPointSet.h>

namespace mitk
{
  /**
   * Class PointSetDataInteractor
   * \brief Implementation of the PointSetInteractor (adapted to new interaction concept)
   *
   * Interactor operates on a point set and supports to:
   * - add points
   * - remove points
   * - move single points
   * - move complete pointset
   * - select/unselect a point
   *
   * in 2d and 3d render windows.
   */

  class MITK_CORE_EXPORT PointSetDataInteractor: public DataInteractor
  {

  public:
    mitkClassMacro(PointSetDataInteractor, DataInteractor);
    itkNewMacro(Self);

  protected:
    PointSetDataInteractor();
    virtual ~PointSetDataInteractor();
    /**
     * Here actions string from the loaded state machine pattern are mapped to functions of
     * the Interactor.
     */
    virtual void ConnectActionsAndFunctions();
    /**
     * This function is called when a DataNode has been set/changed.
     * It is used to initialize the DataNode, e.g. if no PointSet exists yet it is created
     * and added to the DataNode.
     */
    virtual void DataNodeChanged();

    /** Adds a point at the given coordinates.
     *  Every time a point is added it is also checked if the maximal number of points is reached,
     *  and if so an InternalEvent with the signal name "MaxNumberOfPoints" is triggered.
     */
    virtual bool AddPoint(StateMachineAction*, InteractionEvent*);
    /** Removes point that is selected */
    virtual bool RemovePoint(StateMachineAction*, InteractionEvent*);
    /**
     * Checks if new point is close enough to an old one,
     * if so, trigger the ClosedContour signal which can be caught by the state machine.
     */
    virtual bool IsClosedContour(StateMachineAction*, InteractionEvent*);

    virtual bool MovePoint(StateMachineAction*, InteractionEvent*);
    virtual bool InitMove(StateMachineAction*, InteractionEvent*);
    virtual bool FinishMove(StateMachineAction*, InteractionEvent*);
    /**
     * Stores original position from which movement can be calculated
     */
    virtual bool InitMoveAll(StateMachineAction*, InteractionEvent*);
    /**
     * Moves all points of the PointSet by the same vector, which is the relative distance from the
     * initialization point.
     */
    virtual bool MoveSet(StateMachineAction*, InteractionEvent*);
    virtual bool SelectPoint(StateMachineAction*, InteractionEvent*);
    virtual bool UnSelectPoint(StateMachineAction*, InteractionEvent*);
    virtual bool Abort(StateMachineAction*, InteractionEvent*);

  private:

    PointSet::Pointer m_PointSet;
    int m_NumberOfPoints; // to keep track of number of points in pointset
    int m_MaxNumberOfPoints; // maximum of allowed number of points
    int m_SelectedPointIndex; // index of currently selected point in PointSet
    /** member to keep track of PointSet movements */
    Point3D m_LastMovePosition;
  };

}
#endif
