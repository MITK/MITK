#include "mitkPositionTracker.h"

#include <mitkBaseRenderer.h>
#include <mitkDataTree.h>
#include <mitkDisplayPositionEvent.h>
#include "mitkInteractionConst.h"
#include <mitkPointSet.h>
#include <mitkRenderingManager.h>


mitk::PositionTracker::PositionTracker(const char * type, mitk::OperationActor* operationActor)
: mitk::StateMachine(type)
{}


bool mitk::PositionTracker::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
//  std::cout<<"PositionTracker";
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
             
              if( ps->GetPointSet()->GetPoints()->IndexExists( 0 )) //first element
                ps->GetPointSet()->GetPoints()->SetElement( 0, displayPositionEvent->GetWorldPosition());                            
              else
                ps->GetPointSet()->GetPoints()->InsertElement(0,displayPositionEvent->GetWorldPosition());

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
