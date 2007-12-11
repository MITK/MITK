/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <mitkPositionTracker.h>

#include <mitkBaseRenderer.h>
#include <mitkDataTree.h>
#include <mitkDisplayPositionEvent.h>
#include <mitkAction.h>
#include <mitkStateEvent.h>
#include <mitkUndoController.h>
#include <mitkInteractionConst.h>
#include <mitkPointSet.h>
#include <mitkRenderingManager.h>
#include <mitkVector.h> // for PointDataType 


mitk::PositionTracker::PositionTracker(const char * type, mitk::OperationActor* /*operationActor*/)
: mitk::StateMachine(type)
{}


bool mitk::PositionTracker::ExecuteAction(Action* /*action*/, mitk::StateEvent const* stateEvent)
{
  bool ok = false;
  const DisplayPositionEvent* displayPositionEvent = dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());

  mitk::DataTreeIteratorClone dtit;
  mitk::DataTreeNode::Pointer  dtnode;

  if(displayPositionEvent!=NULL)
  {
    if(stateEvent->GetEvent()->GetSender()!=NULL)
    {
      dtit = stateEvent->GetEvent()->GetSender()->GetData();
    }
    else
    {
      itkWarningMacro(<<"StateEvent::GetSender()==NULL - setting timeInMS to 0");
      return false;
    }


    if (dtit.IsNull()) return false;

    // looking for desired point set
    bool isPointSet=false;
    while( !dtit->IsAtEnd() )
    {
      dtnode = dtit->Get();
      if ( dtnode->GetBoolProperty("inputdevice",isPointSet) && isPointSet)
      {
//        mitk::StringProperty sp = new mitk::StringProperty(stateEvent->GetEvent()->GetSender()->GetName());
          int mapperID = stateEvent->GetEvent()->GetSender()->GetMapperID();
          dtnode->SetIntProperty("BaseRendererMapperID",mapperID);

        break;
      }
      ++dtit;
    }

    if ( dtit->IsAtEnd() ) return ok;

    if( isPointSet )
    {
      mitk::PointSet* ps = (mitk::PointSet*) dtnode->GetData();
      int position = 0;

      if( ps->GetPointSet()->GetPoints()->IndexExists( position )) //first element
      {
        ps->GetPointSet()->GetPoints()->SetElement( position, displayPositionEvent->GetWorldPosition());                            
      }
      else
      {
        mitk::PointSet::PointDataType pointData = {position , false /*selected*/, mitk::PTUNDEFINED};
        ps->GetPointSet()->GetPointData()->InsertElement(position, pointData);
        ps->GetPointSet()->GetPoints()->InsertElement(position, displayPositionEvent->GetWorldPosition());
      }

      ps->Modified();

      dtnode->SetData(ps);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

  }
  return ok;
}
