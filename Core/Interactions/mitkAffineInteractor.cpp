#include "mitkAffineInteractor.h"
#include "mitkInteractionConst.h"
#include <mitkDataTreeNode.h>
#include "mitkGeometry3D.h"
#include "mitkAffineTransformationOperation.h"
#include "mitkPositionEvent.h"
#include "vtkTransform.h"
#include <mitkRenderWindow.h>
#include <mitkBoolProperty.h>

#include <itkBoundingBox.h>
#include <itkFixedArray.h>

#include <math.h>

typedef itk::FixedArray< mitk::ScalarType, 3*2 > BoundsArrayType;

mitk::AffineInteractor::AffineInteractor(const char * type, DataTreeNode* dataTreeNode)
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
  case SeCHECKELEMENT:
  { 
    bool selected = false;
    if (m_DataTreeNode->GetBoolProperty("selected", selected) == false)        // if property does not exist
      m_DataTreeNode->SetProperty("selected", new mitk::BoolProperty(false));  // create it  

    mitk::StateEvent* newStateEvent = NULL;

    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) 
      return false;  

		//converting from Point3D to itk::Point
    mitk::Point3D worldPoint = posEvent->GetWorldPosition();

    BoundingBox* box = const_cast <BoundingBox*> (m_DataTreeNode->GetData()->GetGeometry()->GetBoundingBox());
    BoundsArrayType bounds = box->GetBounds();

    ScalarType p[4];
    p[0] = worldPoint.x;
    p[1] = worldPoint.y;
    p[2] = worldPoint.z;
    p[3] = 1;
    geometry->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
		mitk::ITKPoint3D itkPoint;
    itkPoint[0] = p[0]/p[3];
    itkPoint[1] = p[1]/p[3];
    itkPoint[2] = p[2]/p[3];

    itkPoint[0] *= geometry->GetXAxis().GetNorm();
    itkPoint[1] *= geometry->GetYAxis().GetNorm();
    itkPoint[2] *= geometry->GetZAxis().GetNorm();    
    
    
    // check if point is inside the datas bounding box
    if (box->IsInside(itkPoint))
    {
      newStateEvent = new mitk::StateEvent(StYES, posEvent);  
      selected = true;
    } 
    else
    {
      newStateEvent = new mitk::StateEvent(StNO, posEvent);
      selected = false;
    }
    // write new state (selected/not selected) to the property
    mitk::BoolProperty::Pointer prop = new mitk::BoolProperty(selected);
    m_DataTreeNode->GetPropertyList()->SetProperty("selected", prop);
    
    //call HandleEvent to leave the guard-state
    this->HandleEvent( newStateEvent, objectEventId, groupEventId );
		ok = true;
    break;
  }
  case SeTRANSLATESTART:
  {
    // if we are not selected, don't do anything
    //bool selected = false;
    //if (m_DataTreeNode->GetBoolProperty("selected", selected) == false)        // if property does not exist
    //  m_DataTreeNode->SetProperty("selected", new mitk::BoolProperty(false));  // create it  
    //if (selected == false) 
    //{
    //  ok = true;
    //  break;
    //}

    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;    
    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);
    m_LastTranslatePosition = newPosition.GetVectorFromOrigin();
    ok = true;
    break;
  }
  case SeTRANSLATE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;

    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);
    
    newPosition -=  m_LastTranslatePosition;  // compute difference between actual and last mouse position
    
    // create operation with position difference
    AffineTransformationOperation* doOp = new mitk::AffineTransformationOperation(OpMOVE, newPosition, 0.0, 0); // Index is not used here
		if (m_UndoEnabled)	//write to UndoMechanism
		{
      mitk::ScalarType pos[3];
      geometry->GetVtkTransform()->GetPosition(pos);
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

    mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);  // save actual mouse position as last position
    m_LastTranslatePosition = newPosition.GetVectorFromOrigin();

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
    geometry->GetVtkTransform()->GetPosition(position);
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
    m_LastScalePosition = newPosition;
    m_FirstScalePosition = newPosition;

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
    
    mitk::ITKVector3D v = newPosition - m_LastScalePosition;

    // calculate scale changes
    mitk::ITKPoint3D newScale;
    newScale[0] = (geometry->GetXAxis() * v) / geometry->GetXAxis().GetNorm();  // Scalarprodukt of normalized Axis
    newScale[1] = (geometry->GetYAxis() * v) / geometry->GetYAxis().GetNorm();  // and direction vector of mouse movement
    newScale[2] = (geometry->GetZAxis() * v) / geometry->GetZAxis().GetNorm();  // is the length of the movement vectors
                                                                                // projection onto the axis

    // calculate direction of mouse move (towards or away from the data object)
    ITKPoint3D objectPosition = geometry->GetVtkTransform()->GetPosition();
    //ScalarType xdif = fabs((newPosition - objectPosition)[0]) - fabs((m_FirstScalePosition - objectPosition)[0]);
    //ScalarType ydif = fabs((newPosition - objectPosition)[1]) - fabs((m_FirstScalePosition - objectPosition)[1]);
    //ScalarType zdif = fabs((newPosition - objectPosition)[2]) - fabs((m_FirstScalePosition - objectPosition)[2]);
    ScalarType xdif = fabs((newPosition - objectPosition)[0]) - fabs((m_LastScalePosition - objectPosition)[0]);
    ScalarType ydif = fabs((newPosition - objectPosition)[1]) - fabs((m_LastScalePosition - objectPosition)[1]);
    ScalarType zdif = fabs((newPosition - objectPosition)[2]) - fabs((m_LastScalePosition - objectPosition)[2]);

    if(xdif < 0)
      newScale[0] = - fabs(newScale[0]);
    else
      newScale[0] = + fabs(newScale[0]);
    if(ydif < 0)
      newScale[1] = - fabs(newScale[1]);
    else
      newScale[1] = + fabs(newScale[1]);
    if(zdif < 0)
      newScale[2] = - fabs(newScale[2]);
    else
      newScale[2] = + fabs(newScale[2]);
 
    m_LastScalePosition = newPosition;

    AffineTransformationOperation* doOp = new mitk::AffineTransformationOperation(OpSCALE, newScale, 0.0, 0); // Index is not used here

		if (m_UndoEnabled)	//write to UndoMechanism
		{     
      mitk::ITKPoint3D oldScaleData;
      oldScaleData[0] = -newScale[0];
      oldScaleData[1] = -newScale[1];
      oldScaleData[2] = -newScale[2];
      
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
