#include "mitkAffineInteractor.h"
#include "mitkInteractionConst.h"
#include <mitkDataTreeNode.h>
#include "mitkGeometry3D.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
#include "vtkTransform.h"
#include <mitkRenderWindow.h>

mitk::AffineInteractor::AffineInteractor(std::string type, DataTreeNode* dataTreeNode)
	 : Interactor(type, dataTreeNode)
{
  m_ScaleFactor = 1.0;
}

bool mitk::AffineInteractor::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  //std::cout << "AffineInteractor: Execute Sideeffect called " << sideEffectId << std::endl;

	bool ok = false;//for return type bool

  mitk::Geometry3D* geometry = m_DataTreeNode->GetData()->GetGeometry();
	if (geometry == NULL)
		return false;

  /*Each case must watch the type of the event!*/
  switch (sideEffectId)
	{
   case SeNEWPOINT:
  {
    break;
  }
  case SeTRANSLATE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;

    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);
    
    PointOperation* doOp = new mitk::PointOperation(OpMOVE, newPosition, 0); // Index is not used here
		if (m_UndoEnabled)	//write to UndoMechanism
		{
      float pos[3];
      geometry->GetTransform()->GetPosition(pos);
      mitk::ITKPoint3D oldPosition;
      oldPosition[0] = pos[0];
      oldPosition[1] = pos[1];
      oldPosition[2] = pos[2];
      
      PointOperation* undoOp = new mitk::PointOperation(OpMOVE, oldPosition, 0);
			OperationEvent *operationEvent = new OperationEvent(geometry,
																	doOp, undoOp,
																	objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		//execute the Operation
		geometry->ExecuteOperation(doOp);

    ok = true;
	  break;
  }
  case SeROTATESTART:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;    
    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);

    m_lastRotatePosition = newPosition;
    mitk::ScalarType lastOrientation[3];
    geometry->GetTransform()->GetOrientation (lastOrientation);    
    m_lastRotateData = lastOrientation;

    std::cout << "AffineInteractor: Orienatation = <" << lastOrientation[0] << ", " << lastOrientation[1] << ", " << lastOrientation[2] << ">\n";
    ok = true;
    break;
  }
  case SeROTATE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;    
    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);
        
    mitk::ITKPoint3D newOrientation;

    
    // HIER gehts weiter!!! Winkel ausrechnen!

    PointOperation* doOp = new mitk::PointOperation(OpROTATE, newOrientation, 0); // Index is not used here
		if (m_UndoEnabled)	//write to UndoMechanism
		{     
      mitk::ScalarType oldOrientation[3];
      geometry->GetTransform()->GetOrientation(oldOrientation);
      mitk::ITKPoint3D oldOrientationData;
      oldOrientationData = oldOrientation;
      
      PointOperation* undoOp = new mitk::PointOperation(OpROTATE, oldOrientationData, 0);
			OperationEvent *operationEvent = new OperationEvent(geometry,
																	doOp, undoOp,
																	objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		//execute the Operation
		geometry->ExecuteOperation(doOp);

    ok = true;
	  break;
  }
  case SeSCALESTART:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;    
    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);
    m_lastScalePosition = newPosition;
    mitk::ScalarType lastScale[3];
    geometry->GetTransform()->GetScale(lastScale);
    m_lastScaleData = lastScale;
  
    std::cout << "AffineInteractor: scale Start: at <" << m_lastScalePosition[0] << ", " << m_lastScalePosition[1] << ", " << m_lastScalePosition[2] << ">\n";
    ok = true;
    break;
  }
  case SeSCALE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;    
    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);
    
    float oldScale[3];
    geometry->GetTransform()->GetScale(oldScale);
    
    mitk::ITKPoint3D newScale;
    newScale[0] = m_lastScaleData[0] + (newPosition[0] - m_lastScalePosition[0]) * m_ScaleFactor;
    newScale[1] = m_lastScaleData[1] + (newPosition[1] - m_lastScalePosition[1]) * m_ScaleFactor;
    newScale[2] = m_lastScaleData[2] + (newPosition[2] - m_lastScalePosition[2]) * m_ScaleFactor;      

    PointOperation* doOp = new mitk::PointOperation(OpSCALE, newScale, 0); // Index is not used here
		if (m_UndoEnabled)	//write to UndoMechanism
		{     
      mitk::ITKPoint3D oldScaleData;
      oldScaleData[0] = oldScale[0];
      oldScaleData[1] = oldScale[1];
      oldScaleData[2] = oldScale[2];
      
      PointOperation* undoOp = new mitk::PointOperation(OpSCALE, oldScaleData, 0);
			OperationEvent *operationEvent = new OperationEvent(geometry,
																	doOp, undoOp,
																	objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		//execute the Operation
		geometry->ExecuteOperation(doOp);

    ok = true;
	  break;
  }
  default:
    ok = true;
  }
  mitk::RenderWindow::UpdateAllInstances();
  return ok;
}