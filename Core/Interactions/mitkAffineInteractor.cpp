#include "mitkAffineInteractor.h"
#include "mitkInteractionConst.h"
#include <mitkDataTreeNode.h>
#include "mitkGeometry3D.h"
#include "mitkRotationOperation.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
#include "vtkTransform.h"
#include <mitkRenderWindow.h>
#include <mitkBoolProperty.h>

#include <itkBoundingBox.h>
#include <itkFixedArray.h>

#include "mitkBoundingObject.h"

#include <math.h>

mitk::AffineInteractor::AffineInteractor(const char * type, DataTreeNode* dataTreeNode)
	 : Interactor(type, dataTreeNode)
{
}

bool mitk::AffineInteractor::ExecuteAction(int actionId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
	bool ok = false;

  mitk::Geometry3D* geometry = m_DataTreeNode->GetData()->GetGeometry();
	if (geometry == NULL)
		return false;

  /* Each case must watch the type of the event! */
  switch (actionId)
	{
  case AcNEWPOINT:
  {
    //ok = true;
    break;
  }
  case AcCHECKELEMENT:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    mitk::StateEvent* newStateEvent = NULL;
    
    if (posEvent != NULL) //for 2D Interaction
    {
      mitk::Point3D worldPoint = posEvent->GetWorldPosition();

      mitk::BoolProperty::Pointer prop;

      if (this->CheckSelected(worldPoint))
      {
        newStateEvent = new mitk::StateEvent(StYES, posEvent);
        prop = new mitk::BoolProperty(true);
      }
      else
      {
        newStateEvent = new mitk::StateEvent(StNO, posEvent);
        prop = new mitk::BoolProperty(false);
      }

      /* write new state (selected/not selected) to the property */
      m_DataTreeNode->GetPropertyList()->SetProperty("selected", prop);
    }
    else // DisplayEvent from 3D Window or else Event
    {
      newStateEvent = new mitk::StateEvent(StNO, posEvent);
      //no change on selected state because of no interaction in 3D right now
    }
    this->HandleEvent( newStateEvent, objectEventId, groupEventId );
		ok = true;
    break;
  }
  case AcADD:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL)
      return false;
    mitk::Point3D worldPoint = posEvent->GetWorldPosition();

    mitk::StateEvent* newStateEvent = NULL;
    if (this->CheckSelected(worldPoint))
    {
      newStateEvent = new mitk::StateEvent(StYES, posEvent);
      m_DataTreeNode->GetPropertyList()->SetProperty("selected", new mitk::BoolProperty(true));
    }
    else  // if not selected, do nothing (don't deselect)
    {
      newStateEvent = new mitk::StateEvent(StNO, posEvent);
    }
    //call HandleEvent to leave the guard-state
    this->HandleEvent( newStateEvent, objectEventId, groupEventId );
		ok = true;
    break;
  }
  case AcTRANSLATESTART:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;
    //converting from Point3D to itk::Point
		//mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), m_LastMousePosition);
    //m_LastTranslatePosition = newPosition.GetVectorFromOrigin();
    ok = true;
    break;
  }
  case AcTRANSLATE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;

    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);

    newPosition -=  m_LastMousePosition.GetVectorFromOrigin();  // compute difference between actual and last mouse position

    mitk::vm2itk(posEvent->GetWorldPosition(), m_LastMousePosition);  // save current mouse position as last position

    /* create operation with position difference */
    mitk::PointOperation* doOp = new mitk::PointOperation(OpMOVE, newPosition, 0); // Index is not used here
		if (m_UndoEnabled)	//write to UndoMechanism
		{
      mitk::ScalarType pos[3];
      geometry->GetVtkTransform()->GetPosition(pos);
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
		/* execute the Operation */
		geometry->ExecuteOperation(doOp);
    ok = true;
	  break;
  }
  case AcROTATESTART:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;
    //converting from Point3D to itk::Point
		//mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), m_LastMousePosition);
    // save mouse down position
    //m_lastRotatePosition = newPosition.GetVectorFromOrigin();
    ok = true;
    break;
  }
  case AcROTATE:
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

    newPosition -= dataPosition;  // calculate vector from center of the data object to the current mouse position

    mitk::ITKVector3D startPosition = m_LastMousePosition.GetVectorFromOrigin() - dataPosition;  // calculate vector from center of the data object to the last mouse position

    /* calculate rotation axis (by calculating the cross produkt of the vectors) */
    mitk::ITKVector3D rotationaxis;   
    rotationaxis[0] =  startPosition[1] * newPosition[2] - startPosition[2] * newPosition[1];
    rotationaxis[1] =  startPosition[2] * newPosition[0] - startPosition[0] * newPosition[2];
    rotationaxis[2] =  startPosition[0] * newPosition[1] - startPosition[1] * newPosition[0];

    /* calculate rotation angle in degrees */
    mitk::ScalarType angle = atan2(rotationaxis.GetNorm(), newPosition * startPosition) * (180/vnl_math::pi);

    m_LastMousePosition = dummy; // save current mouse position as last mouse position

    /* create operation with center of rotation, angle and axis and send it to the geometry and Undo controller */
    mitk::RotationOperation* doOp = new mitk::RotationOperation(OpROTATE, ITKPoint3D(position) , rotationaxis, angle);

    if (m_UndoEnabled)	//write to UndoMechanism
		{
      RotationOperation* undoOp = new mitk::RotationOperation(OpROTATE, ITKPoint3D(position) , rotationaxis, -angle);
			OperationEvent *operationEvent = new OperationEvent(geometry,
																	doOp, undoOp,
																	objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		/* execute the Operation */
		geometry->ExecuteOperation(doOp);
    ok = true;
	  break;
  }
  case AcSCALESTART:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;
    /*converting from Point3D to itk::Point */
		mitk::vm2itk(posEvent->GetWorldPosition(), m_LastMousePosition);

    ok = true;
    break;
  }
  case AcSCALE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) return false;
    //converting from Point3D to itk::Point
		mitk::ITKPoint3D newPosition;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);

    mitk::ITKVector3D v = newPosition - m_LastMousePosition;
    /* calculate scale changes */
    mitk::ITKPoint3D newScale;
    newScale[0] = (geometry->GetXAxis() * v) / geometry->GetXAxis().GetNorm();  // Scalarprodukt of normalized Axis
    newScale[1] = (geometry->GetYAxis() * v) / geometry->GetYAxis().GetNorm();  // and direction vector of mouse movement
    newScale[2] = (geometry->GetZAxis() * v) / geometry->GetZAxis().GetNorm();  // is the length of the movement vectors
                                                                                // projection onto the axis
    /* convert movement to local object coordinate system and mirror it to the positive quadrant */
    ITKVector3D start;
    ITKVector3D end;
    mitk::ScalarType convert[3];
    itk2vtk(m_LastMousePosition, convert);
    geometry->GetVtkTransform()->GetInverse()->TransformPoint(convert, convert);  // transform start point to local object coordinates
    start[0] = fabs(convert[0]);  start[1] = fabs(convert[1]);  start[2] = fabs(convert[2]);  // mirror it to the positive quadrant
    itk2vtk(newPosition, convert);
    geometry->GetVtkTransform()->GetInverse()->TransformPoint(convert, convert);  // transform end point to local object coordinates
    end[0] = fabs(convert[0]);  end[1] = fabs(convert[1]);  end[2] = fabs(convert[2]);  // mirror it to the positive quadrant

    /* check if mouse movement is towards or away from the objects axes and adjust scale factors accordingly */
    ITKVector3D vLocal = start - end;
    newScale[0] = (vLocal[0] > 0.0) ? -fabs(newScale[0]) : +fabs(newScale[0]);
    newScale[1] = (vLocal[1] > 0.0) ? -fabs(newScale[1]) : +fabs(newScale[1]);
    newScale[2] = (vLocal[2] > 0.0) ? -fabs(newScale[2]) : +fabs(newScale[2]);
    
    m_LastMousePosition = newPosition;  // update lastPosition for next mouse move

    /* generate Operation and send it to the receiving geometry */
    PointOperation* doOp = new mitk::PointOperation(OpSCALE, newScale, 0); // Index is not used here
		if (m_UndoEnabled)	//write to UndoMechanism
		{
      mitk::ITKPoint3D oldScaleData;
      oldScaleData[0] = -newScale[0];
      oldScaleData[1] = -newScale[1];
      oldScaleData[2] = -newScale[2];

      PointOperation* undoOp = new mitk::PointOperation(OpSCALE, oldScaleData, 0);
			OperationEvent *operationEvent = new OperationEvent(geometry,
																	doOp, undoOp,
																	objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		/* execute the Operation */
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

bool mitk::AffineInteractor::CheckSelected(const mitk::Point3D& worldPoint)
{
    bool selected = false;
    if (m_DataTreeNode->GetBoolProperty("selected", selected) == false)        // if property does not exist
      m_DataTreeNode->SetProperty("selected", new mitk::BoolProperty(false));  // create it

    // check if mouseclick has hit the object
    mitk::ITKPoint3D itkPoint;
    mitk::BoundingObject::Pointer boundingObject = dynamic_cast<mitk::BoundingObject*>(m_DataTreeNode->GetData());
    if(boundingObject.IsNotNull())  // if it is a bounding object, use its inside function for exact hit calculation
    {
      itkPoint[0] = worldPoint.x;
      itkPoint[1] = worldPoint.y;
      itkPoint[2] = worldPoint.z;
      selected = boundingObject->IsInside(itkPoint); // check if point is inside the object
    }
    else    // use the data objects bounding box to determine if hit
    {
      m_DataTreeNode->GetData()->UpdateOutputInformation(); // update bounding box @TODO: Is this neccessary?
      BoundingBox* box = const_cast <BoundingBox*> (m_DataTreeNode->GetData()->GetGeometry()->GetBoundingBox());
      ScalarType p[4];
      p[0] = worldPoint.x;
      p[1] = worldPoint.y;
      p[2] = worldPoint.z;
      p[3] = 1;
      m_DataTreeNode->GetData()->GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
      itkPoint[0] = p[0]/p[3];
      itkPoint[1] = p[1]/p[3];
      itkPoint[2] = p[2]/p[3];
      //// reaply scaling, because bounding box is scaled
      //itkPoint[0] *= m_DataTreeNode->GetData()->GetGeometry()->GetXAxis().GetNorm();
      //itkPoint[1] *= m_DataTreeNode->GetData()->GetGeometry()->GetYAxis().GetNorm();
      //itkPoint[2] *= m_DataTreeNode->GetData()->GetGeometry()->GetZAxis().GetNorm();
      selected = box->IsInside(itkPoint); // check if point is inside the datas bounding box
    }
    return selected;
}
