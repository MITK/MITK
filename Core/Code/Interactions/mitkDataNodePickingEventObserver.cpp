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

#include "mitkDataNodePickingEventObserver.h"

#include "mitkMousePressEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseReleaseEvent.h"

#include "mitkVtkPropRenderer.h"

namespace mitk {

DataNodePickingEventObserver::DataNodePickingEventObserver()
{
}

DataNodePickingEventObserver::~DataNodePickingEventObserver()
{
}

void DataNodePickingEventObserver::HandlePickOneEvent(InteractionEvent* interactionEvent)
{
    SingleNodePickEvent.Send(GetPickedDataNode(interactionEvent));
}

void DataNodePickingEventObserver::HandlePickAddEvent(InteractionEvent* interactionEvent)
{
    AddNodePickEvent.Send(GetPickedDataNode(interactionEvent));
}

void DataNodePickingEventObserver::HandlePickToggleEvent(InteractionEvent* interactionEvent)
{
    ToggleNodePickEvent.Send(GetPickedDataNode(interactionEvent));
}

mitk::DataNode* DataNodePickingEventObserver::GetPickedDataNode(InteractionEvent* interactionEvent)
{
    const mitk::MouseReleaseEvent* releaseEvent = static_cast<const mitk::MouseReleaseEvent*>(interactionEvent);

    mitk::VtkPropRenderer* propRenderer = static_cast<mitk::VtkPropRenderer*>(interactionEvent->GetSender());

    mitk::Point3D p3d;
    return propRenderer->PickObject(releaseEvent->GetPointerPositionOnScreen(), p3d);
}

}
