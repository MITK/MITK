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

#include "mitkUSZonesInteractor.h"
#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkSurface.h"
#include "mitkInteractionPositionEvent.h"

#include <vtkSphereSource.h>

const char* mitk::USZonesInteractor::DATANODE_PROPERTY_SIZE = "zone.size";
const char* mitk::USZonesInteractor::DATANODE_PROPERTY_CREATED = "zone.created";

void mitk::USZonesInteractor::UpdateSurface(mitk::DataNode::Pointer dataNode)
{
  if (!dataNode->GetData())
  {
    MITK_WARN("USZonesInteractor")("DataInteractor")
      << "Cannot update surface for node as no data is set to the node.";
    return;
  }

  mitk::Point3D origin = dataNode->GetData()->GetGeometry()->GetOrigin();

  float radius;
  if (!dataNode->GetFloatProperty(DATANODE_PROPERTY_SIZE, radius))
  {
    MITK_WARN("USZonesInteractor")("DataInteractor")
      << "Cannut update surface for node as no radius is specified in the node properties.";
    return;
  }

  mitk::Surface::Pointer zone = mitk::Surface::New();

  // create a vtk sphere with given radius
  vtkSmartPointer<vtkSphereSource> vtkSphere = vtkSmartPointer<vtkSphereSource>::New();
  vtkSphere->SetRadius(radius);
  vtkSphere->SetCenter(0, 0, 0);
  vtkSphere->SetPhiResolution(20);
  vtkSphere->SetThetaResolution(20);
  vtkSphere->Update();
  zone->SetVtkPolyData(vtkSphere->GetOutput());

  // set vtk sphere and origin to data node (origin must be set
  // again, because of the new sphere set as data)
  dataNode->SetData(zone);
  dataNode->GetData()->GetGeometry()->SetOrigin(origin);

  // update the RenderWindow to show the changed surface
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::USZonesInteractor::USZonesInteractor()
{
}

mitk::USZonesInteractor::~USZonesInteractor()
{
}

void mitk::USZonesInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("addCenter", AddCenter);
  CONNECT_FUNCTION("changeRadius", ChangeRadius);
  CONNECT_FUNCTION("endCreation", EndCreation);
  CONNECT_FUNCTION("abortCreation", AbortCreation);
}

void mitk::USZonesInteractor::DataNodeChanged()
{
  mitk::DataNode::Pointer dataNode = this->GetDataNode();
  if (dataNode.IsNotNull() && dataNode->GetData() == 0)
  {
    dataNode->SetData(mitk::Surface::New());
  }
}

void mitk::USZonesInteractor::AddCenter(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  // cast InteractionEvent to a position event in order to read out the mouse position
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  mitk::DataNode::Pointer dataNode = this->GetDataNode();
  dataNode->SetBoolProperty(DATANODE_PROPERTY_CREATED, false);

  // make sure that data node contains data
  mitk::BaseData::Pointer dataNodeData = this->GetDataNode()->GetData();
  if (dataNodeData.IsNull())
  {
    dataNodeData = mitk::Surface::New();
    this->GetDataNode()->SetData(dataNodeData);
  }

  // set origin of the data node to the mouse click position
  dataNodeData->GetGeometry()->SetOrigin(positionEvent->GetPositionInWorld());
  MITK_INFO("USNavigationLogging") << "Critical Structure added on position " << positionEvent->GetPointerPositionOnScreen() << " (Image Coordinates); "
    << positionEvent->GetPositionInWorld() << " (World Coordinates)";

  dataNode->SetFloatProperty("opacity", 0.60f);

  //return true;
}

void mitk::USZonesInteractor::ChangeRadius(mitk::StateMachineAction*, mitk::InteractionEvent* interactionEvent)
{
  // cast InteractionEvent to a position event in order to read out the mouse position
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  mitk::DataNode::Pointer curNode = this->GetDataNode();
  mitk::Point3D mousePosition = positionEvent->GetPositionInWorld();

  mitk::ScalarType radius = mousePosition.EuclideanDistanceTo(curNode->GetData()->GetGeometry()->GetOrigin());
  curNode->SetFloatProperty(DATANODE_PROPERTY_SIZE, radius);

  mitk::USZonesInteractor::UpdateSurface(curNode);

  //return true;
}

void mitk::USZonesInteractor::EndCreation(mitk::StateMachineAction*, mitk::InteractionEvent* /*interactionEvent*/)
{
  this->GetDataNode()->SetBoolProperty(DATANODE_PROPERTY_CREATED, true);
  //return true;
}

void mitk::USZonesInteractor::AbortCreation(mitk::StateMachineAction*, mitk::InteractionEvent*)
{
  this->GetDataNode()->SetData(mitk::Surface::New());

  // update the RenderWindow to remove the surface
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  //return true;
}
