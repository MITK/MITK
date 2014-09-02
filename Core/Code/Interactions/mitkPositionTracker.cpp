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

#include <mitkPositionTracker.h>

#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkNodePredicateProperty.h>
#include <mitkDisplayPositionEvent.h>
#include <mitkAction.h>
#include <mitkStateEvent.h>
#include <mitkUndoController.h>
#include <mitkInteractionConst.h>
#include <mitkPointSet.h>
#include <mitkRenderingManager.h>
#include <mitkNumericTypes.h> // for PointDataType
#include <mitkProperties.h>


mitk::PositionTracker::PositionTracker(const char * type, mitk::OperationActor* /*operationActor*/)
: mitk::StateMachine(type)
{}


bool mitk::PositionTracker::ExecuteAction(Action* /*action*/, mitk::StateEvent const* stateEvent)
{
  bool ok = false;
  const DisplayPositionEvent* displayPositionEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());

  mitk::DataNode::Pointer  dtnode;
  mitk::DataStorage* ds = NULL;
  if (displayPositionEvent == NULL)
    return false;

  if (stateEvent->GetEvent()->GetSender()!=NULL)
  {
    ds = stateEvent->GetEvent()->GetSender()->GetDataStorage();
  }
  else
  {
    itkWarningMacro(<<"StateEvent::GetSender()==NULL - setting timeInMS to 0");
    return false;
  }
  if (ds == NULL)
    return false;

  // looking for desired point set
  dtnode = ds->GetNode(mitk::NodePredicateProperty::New("inputdevice", mitk::BoolProperty::New(true)));
  if (dtnode.IsNull())
    return false;

  dtnode->SetIntProperty("BaseRendererMapperID", stateEvent->GetEvent()->GetSender()->GetMapperID());
  mitk::PointSet* ps = dynamic_cast<mitk::PointSet*>(dtnode->GetData());
  if (ps == NULL)
    return false;

  int position = 0;
  if( ps->GetPointSet()->GetPoints()->IndexExists( position )) //first element
  {
    ps->GetPointSet()->GetPoints()->SetElement( position, displayPositionEvent->GetWorldPosition());
  }
  else
  {
    mitk::PointSet::PointDataType pointData = {static_cast<unsigned int>(position) , false /*selected*/, mitk::PTUNDEFINED};
    ps->GetPointSet()->GetPointData()->InsertElement(position, pointData);
    ps->GetPointSet()->GetPoints()->InsertElement(position, displayPositionEvent->GetWorldPosition());
  }
  ps->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return ok;
}
