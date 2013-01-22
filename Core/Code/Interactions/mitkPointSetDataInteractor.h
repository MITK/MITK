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
   * \brief Implementation of the PointSetInteractor using the new interaction concept
   *
   *TODO rest of expl
   */

  class MITK_CORE_EXPORT PointSetDataInteractor: public DataInteractor
  {

  public:
    mitkClassMacro(PointSetDataInteractor, DataInteractor);
    itkNewMacro(Self);

  protected:
    PointSetDataInteractor();
    virtual ~PointSetDataInteractor();
    virtual void ConnectActionsAndFunctions();
    virtual void DataNodeChanged();

    /* PointSet Interactor functionality */
    virtual bool AddPoint(StateMachineAction*, InteractionEvent*);
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
    int m_NumberOfPoints;
    int m_MaxNumberOfPoints;
    int m_SelectedPointIndex;
    /** member to keep track of PointSet movements */
    Point3D m_LastMovePosition;
  };

}
#endif
