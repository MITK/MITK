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


bool mitk::PositionTracker::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
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
       //std::cout<<"PointSet found!!!"<<std::endl;
       break;
     }
     ++dtit;
   }

   if ( dtit->IsAtEnd() ) return ok;

   //switch could be removed if no proper distinction is needed
   switch (action->GetActionId())
   {
     case AcNEWPOINT:
     case AcINITMOVEMENT:
     case AcMOVEPOINT:
     case AcMOVE:
     case AcFINISHMOVEMENT:
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

              dtnode->SetData(ps);
              mitk::RenderingManager::GetInstance()->RequestUpdateAll();
            }
            
            ok = true;
            break;
     case AcREMOVEPOINT:
       //rmv
       std::cout<<"PosTrack: AcREMOVEPOINT"<<std::endl;
     default:
       ok = false;
       break;
   }
   
 }
  return ok;
}
