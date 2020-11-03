/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSPointMarkInteractor.h"
#include "mitkDataNode.h"
#include "mitkInteractionPositionEvent.h"

//#include "mitkPlanarCircle.h"
#include "mitkSurface.h"
//#include "vtkSphere.h"
//#include "vtkSphereSource.h"

mitk::USPointMarkInteractor::USPointMarkInteractor()
{
}

mitk::USPointMarkInteractor::~USPointMarkInteractor()
{
}

void mitk::USPointMarkInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("addPoint", AddPoint);
}

void mitk::USPointMarkInteractor::DataNodeChanged()
{
  mitk::DataNode::Pointer dataNode = this->GetDataNode();
  if (dataNode.IsNotNull() && dataNode->GetData() == nullptr)
  {
    dataNode->SetData(mitk::Surface::New());
  }
}

void mitk::USPointMarkInteractor::AddPoint(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  // cast InteractionEvent to a position event in order to read out the mouse position
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  // set origin of the data node to the mouse click position
  this->GetDataNode()->GetData()->GetGeometry()->SetOrigin(positionEvent->GetPositionInWorld());

  CoordinatesChangedEvent.Send(this->GetDataNode());

  //return true;
}
