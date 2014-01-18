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

#include "mitkUSPointMarkInteractor.h"
#include "mitkDataNode.h"
#include "mitkInteractionPositionEvent.h"

#include "mitkPlanarCircle.h"
#include "mitkSurface.h"
#include "vtkSphere.h"
#include "vtkSphereSource.h"

void mitk::USPointMarkInteractor::UpdateSurface(mitk::DataNode::Pointer dataNode)
{
  if ( ! dataNode->GetData())
  {
    MITK_WARN("USZonesInteractor")("DataInteractor")
        << "Cannot update surface for node as no data is set to the node.";
    return;
  }

  mitk::Point3D origin = dataNode->GetData()->GetGeometry()->GetOrigin();

  mitk::ScalarType radius = 5;
  /*if ( ! dataNode->GetFloatProperty(DATANODE_PROPERTY_SIZE, radius) )
  {
    MITK_WARN("USZonesInteractor")("DataInteractor")
        << "Cannut update surface for node as no radius is specified in the node properties.";
    return;
  }*/

  mitk::Surface::Pointer zone = mitk::Surface::New();

  // create a vtk sphere with given radius
  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius( radius );
  vtkData->SetCenter(0,0,0);
  vtkData->Update();
  zone->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();

  // set vtk sphere and origin to data node (origin must be set
  // again, because of the new sphere set as data)
  dataNode->SetData(zone);
  dataNode->GetData()->GetGeometry()->SetOrigin(origin);

  // update the RenderWindow to show the changed surface
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

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
  if ( dataNode.IsNotNull() && dataNode->GetData() == 0 )
  {
    //dataNode->SetData(mitk::PlanarCircle::New());
    dataNode->SetData(mitk::Surface::New());
  }
}

bool mitk::USPointMarkInteractor::AddPoint(mitk::StateMachineAction* , mitk::InteractionEvent* interactionEvent)
{
  // cast InteractionEvent to a position event in order to read out the mouse position
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL) { return false; }

  //mitk::PlanarCircle::Pointer circle = dynamic_cast<mitk::PlanarCircle*>(this->GetDataNode()->GetData());

  // set origin of the data node to the mouse click position
  this->GetDataNode()->GetData()->GetGeometry()->SetOrigin(positionEvent->GetPositionInWorld());

  mitk::ScalarType rgb[3] = {0.5, 1, 1};
  this->GetDataNode()->SetColor(rgb);
  mitk::USPointMarkInteractor::UpdateSurface(this->GetDataNode());

  /*circle->SetControlPoint(0, positionEvent->GetPositionInWorld());
  circle->SetControlPoint(1, positionEvent->GetPositionInWorld());*/

  CoordinatesChangedEvent.Send(this->GetDataNode());

  return true;
}
