#include "mitkAffineInteractor.h"
#include "mitkInteractionConst.h"
#include <mitkDataTreeNode.h>
#include "mitkGeometry3D.h"
#include "mitkAffineTransformationOperation.h"
#include "mitkPositionEvent.h"
#include "vtkTransform.h"
#include <mitkRenderWindow.h>

#include <math.h>

mitk::AffineInteractor::AffineInteractor(std::string type, DataTreeNode* dataTreeNode)
	 : Interactor(type, dataTreeNode)
{
  //m_ScaleFactor = 1.0;
}

bool mitk::AffineInteractor::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
	bool ok = false;//for return type bool

  mitk::Geometry3D* geometry = m_DataTreeNode->GetData()->GetGeometry();
	if (geometry == NULL)
		return false;

  /* Each case must watch the type of the event! */
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
    
    // create operation with new position
    AffineTransformationOperation* doOp = new mitk::AffineTransformationOperation(OpMOVE, newPosition, 0.0, 0); // Index is not used here
		if (m_UndoEnabled)	//write to UndoMechanism
		{
      mitk::ScalarType pos[3];
      geometry->GetTransform()->GetPosition(pos);
      mitk::ITKPoint3D oldPosition;
      oldPosition[0] = pos[0];
      oldPosition[1] = pos[1];
      oldPosition[2] = pos[2];
      
      AffineTransformationOperation* undoOp = new mitk::AffineTransformationOperation(OpMOVE, oldPosition, 0.0, 0);
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
    // save mouse down position
    m_lastRotatePosition = newPosition.GetVectorFromOrigin();
    ok = true;
    break;
  }
  case SeROTATE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;    
    //converting from Point3D to itk::Point
		mitk::ITKPoint3D dummy;    
    mitk::vm2itk(posEvent->GetWorldPosition(), dummy);

    mitk::ITKVector3D newPosition = dummy.GetVectorFromOrigin();
		      
    mitk::ScalarType position[3];
    geometry->GetTransform()->GetPosition(position);
    mitk::ITKVector3D dataPosition = position;

    newPosition -= dataPosition;  // calculate vector from center of the data object to the mouse position
    
    mitk::ITKVector3D startPosition = m_lastRotatePosition - dataPosition;  // calculate vector from center of the data object to the last mouse position              

    mitk::ITKVector3D rotationaxis;   // calculate rotation axis (by calculating the cross produkt of the vectors)
    rotationaxis[0] =  startPosition[1] * newPosition[2] - startPosition[2] * newPosition[1];
    rotationaxis[1] =  startPosition[2] * newPosition[0] - startPosition[0] * newPosition[2];
    rotationaxis[2] =  startPosition[0] * newPosition[1] - startPosition[1] * newPosition[0];
    
    mitk::ScalarType angle = atan2(rotationaxis.GetNorm(), newPosition * startPosition) * (180/vnl_math::pi);
    
    m_lastRotatePosition = dummy.GetVectorFromOrigin(); // save actual mouse position as last mouse position

    dummy[0] = rotationaxis[0];  // encapsulate rotation vector into a ITKPoint3D for the Operation
    dummy[1] = rotationaxis[1];
    dummy[2] = rotationaxis[2];
    
    // create operation with angle and axis
    AffineTransformationOperation* doOp = new mitk::AffineTransformationOperation(OpROTATE, dummy, angle, 0); // Index is not used here
		
    if (m_UndoEnabled)	//write to UndoMechanism
		{           
      AffineTransformationOperation* undoOp = new mitk::AffineTransformationOperation(OpROTATE, dummy, -angle, 0); // Index is not used here
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
  
    const mitk::ScalarType* lastScale = geometry->GetScale();
    m_lastScaleData[0] = lastScale[0];
    m_lastScaleData[1] = lastScale[1];
    m_lastScaleData[2] = lastScale[2];
 
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
    
          //std::cout << "newPosition:  <" << newPosition[0] <<", " << newPosition[1] << ", " << newPosition[2] << ">\n";    
          //std::cout << "m_lastScalePosition:  <" << m_lastScalePosition[0] <<", " << m_lastScalePosition[1] << ", " << m_lastScalePosition[2] << ">\n";    
    
    mitk::ITKVector3D v = newPosition - m_lastScalePosition;
    //const ScalarType* pos = geometry->GetPosition();
    //v[0] -= pos[0];
    //v[1] -= pos[1];
    //v[2] -= pos[2];

          //std::cout << "v:  <" << v[0] <<", " << v[1] << ", " << v[2] << ">\n";
          //std::cout << "m_lastScaleData:  <" << m_lastScaleData[0] <<", " << m_lastScaleData[1] << ", " << m_lastScaleData[2] << ">\n";

    mitk::ITKPoint3D newScale;
    newScale[0] = m_lastScaleData[0] + (geometry->GetXAxis() * v);  // Scalarprodukt of normalized Axis 
    newScale[1] = m_lastScaleData[1] + (geometry->GetYAxis() * v);  // and direction vector of mouse movement 
    newScale[2] = m_lastScaleData[2] + (geometry->GetZAxis() * v);  // is the length of the movement vectors 
                                                                    // projection onto the axis
    
          //std::cout << "newScale:  <" << newScale[0] <<", " << newScale[1] << ", " << newScale[2] << ">\n";
    
    newScale[0] = (newScale[0] < 1) ? 1 : newScale[0]; // cap at a minimum scale factor of 1
    newScale[1] = (newScale[1] < 1) ? 1 : newScale[1];
    newScale[2] = (newScale[2] < 1) ? 1 : newScale[2];

    AffineTransformationOperation* doOp = new mitk::AffineTransformationOperation(OpSCALE, newScale, 0.0, 0); // Index is not used here

		if (m_UndoEnabled)	//write to UndoMechanism
		{     
      mitk::ScalarType oldScale[3];
      geometry->GetTransform()->GetScale(oldScale);   // Achtung, umstellen!
      mitk::ITKPoint3D oldScaleData = oldScale;
      
      AffineTransformationOperation* undoOp = new mitk::AffineTransformationOperation(OpSCALE, oldScaleData, 0.0, 0);
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
