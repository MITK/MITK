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

#include "mitkSinglePointDataInteractor.h"
#include "mitkMouseMoveEvent.h"

#include "mitkInteractionConst.h" // TODO: refactor file
#include "mitkInternalEvent.h"
#include "mitkOperationEvent.h"
#include "mitkRenderingManager.h"
#include <mitkPointOperation.h>
//
#include "mitkBaseRenderer.h"
#include "mitkDispatcher.h"

#include "mitkUndoController.h"

mitk::SinglePointDataInteractor::SinglePointDataInteractor()
{
  this->SetMaxPoints(1);
}

mitk::SinglePointDataInteractor::~SinglePointDataInteractor()
{
}

void mitk::SinglePointDataInteractor::AddPoint(StateMachineAction * /*stateMachineAction*/,
                                               InteractionEvent *interactionEvent)
{
  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  ScalarType timeInMs = interactionEvent->GetSender()->GetTime();

  // To add a point the minimal information is the position, this method accepts all InteractionsPositionEvents
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent != nullptr)
  {
    PointOperation *doOp;
    PointOperation *undoOp;

    if (m_PointSet->IndexExists(0, timeStep))
    {
      PointSet::PointType pt = m_PointSet->GetPoint(0, timeStep);
      Point3D itkPoint;
      itkPoint[0] = pt[0];
      itkPoint[1] = pt[1];
      itkPoint[2] = pt[2];

      doOp = new mitk::PointOperation(OpMOVE, timeInMs, positionEvent->GetPositionInWorld(), 0);
      undoOp = new mitk::PointOperation(OpMOVE, timeInMs, itkPoint, 0);
    }
    else
    {
      doOp = new mitk::PointOperation(OpINSERT, timeInMs, positionEvent->GetPositionInWorld(), 0);
      undoOp = new mitk::PointOperation(OpREMOVE, timeInMs, positionEvent->GetPositionInWorld(), 0);
    }

    if (m_UndoEnabled)
    {
      OperationEvent *operationEvent = new OperationEvent(m_PointSet, doOp, undoOp, "Move point");
      OperationEvent::IncCurrObjectEventId();

      m_UndoController->SetOperationEvent(operationEvent);
    }
    // execute the Operation
    m_PointSet->ExecuteOperation(doOp);

    if (!m_UndoEnabled)
      delete doOp;

    // Request update
    interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
  }
}

/*
 * Check whether the DataNode contains a pointset, if not create one and add it.
 */
void mitk::SinglePointDataInteractor::DataNodeChanged()
{
  if (GetDataNode() != nullptr)
  {
    auto *points = dynamic_cast<PointSet *>(GetDataNode()->GetData());
    if (points == nullptr)
    {
      m_PointSet = PointSet::New();
      GetDataNode()->SetData(m_PointSet);
    }
    else
    {
      points->Clear();
      m_PointSet = points;
    }
  }
}
