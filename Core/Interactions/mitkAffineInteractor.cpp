#include "mitkAffineInteractor.h"
#include "mitkInteractionConst.h"
#include <mitkDataTreeNode.h>
#include "mitkGeometry3D.h"
#include "mitkRotationOperation.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
#include "mitkDisplayPositionEvent.h"
#include "vtkTransform.h"
#include <mitkRenderWindow.h>
#include <mitkProperties.h>
#include <itkBoundingBox.h>
#include <itkFixedArray.h>
#include <mitkAction.h>

#include "mitkBoundingObject.h"

#include <math.h>

#include <vtkWorldPointPicker.h>
#include <vtkPicker.h>
#include "mitkOpenGLRenderer.h"
#include "mitkBaseRenderer.h"
#include "mitkGlobalInteraction.h"
#include <mitkFocusManager.h>
#include <mitkEventMapper.h>
#include "vtkProp3D.h"
#include "mitkVtkInteractorCameraController.h"
#include <vtkInteractorObserver.h>
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include <vtkInteractorObserver.h>


mitk::AffineInteractor::AffineInteractor(const char * type, DataTreeNode* dataTreeNode)
: Interactor(type, dataTreeNode)
{
}

bool mitk::AffineInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;

  mitk::Geometry3D* geometry = m_DataTreeNode->GetData()->GetGeometry();
  if (geometry == NULL)
    return false;
  /* Position events - 3D coordinates from a 2D window */
  mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
  /* Display  events - 2D coordinates from a 3D window */
  mitk::DisplayPositionEvent const  *displayEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
  /* Each case must watch the type of the event! */
  switch (action->GetActionId())
  {
    case AcINITAFFINEINTERACTIONS:
      {
        ok = true;
        break;
      }
    case AcFINISHAFFINEINTERACTIONS:
      {
        ok = true;
        break;
      }
    case AcNEWPOINT:
      {
        //ok = true;
        break;
      }
    case AcCHECKELEMENT:
      {
        mitk::Point3D worldPoint;
        if (posEvent != NULL)   // Event in 2D Window with 3D coordinates
        {
          worldPoint = posEvent->GetWorldPosition();
        }
        else if (displayEvent != NULL)  // Event in 3D window with 2D coordinates
        {  
          /* pick a Prop3D and assume its position as event 3D coordinates */
          const mitk::Point2D displayPoint = displayEvent->GetDisplayPosition();
          mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
          if (globalInteraction == NULL)
            return false;
          const FocusManager::FocusElement* fe = globalInteraction->GetFocus();
          FocusManager::FocusElement* fe2 =  const_cast <FocusManager::FocusElement*>(fe);
          mitk::OpenGLRenderer* glRenderer = dynamic_cast<mitk::OpenGLRenderer*>(fe2);
          if (glRenderer == NULL)
            return false;
          mitk::Point3D objectPoint;
          vtkPicker* picker =	vtkPicker::New();
          picker->SetTolerance (0.0001);
          picker->Pick(displayPoint.x, displayPoint.y, 0, glRenderer->GetVtkRenderer());
          if (picker->GetProp3D() == NULL)
            return false;
          worldPoint.x = picker->GetPickPosition()[0];
          worldPoint.y = picker->GetPickPosition()[1];
          worldPoint.z = picker->GetPickPosition()[2];
        }
        else  // neither 2D nor 3D coordinates available
          return false;

        /* now we have a worldpoint. check if it is inside our object and select/deselect it accordingly */
        mitk::BoolProperty::Pointer selected;
        mitk::ColorProperty::Pointer color;
        mitk::StateEvent* newStateEvent = NULL;
        if (this->CheckSelected(worldPoint))
        {
          newStateEvent = new mitk::StateEvent(StYES, posEvent);
          selected = new mitk::BoolProperty(true);
          color = new mitk::ColorProperty(1.0, 1.0, 0.0); // if selected, color is yellow
        }
        else
        {
          newStateEvent = new mitk::StateEvent(StNO, posEvent);
          selected = new mitk::BoolProperty(false);
          mitk::BoundingObject* b = dynamic_cast<mitk::BoundingObject*>(m_DataTreeNode->GetData());
          if(b != NULL)
          {
            color = (b->GetPositive())? new mitk::ColorProperty(1.0, 0.0, 0.0) : new mitk::ColorProperty(0.0, 0.0, 1.0);  // if deselected, a boundingobject is colored according to its positive/negative state
          }
          else
            color = new mitk::ColorProperty(1.0, 1.0, 1.0);   // if deselcted and no bounding object, color is white
        }

        /* write new state (selected/not selected) to the property */
        m_DataTreeNode->GetPropertyList()->SetProperty("selected", selected);
        m_DataTreeNode->GetPropertyList()->SetProperty("color", color);
        this->HandleEvent( newStateEvent, objectEventId, groupEventId );
        ok = true;
        break;
      }
    case AcADD:
      {
        if (posEvent == NULL)   // @TODO: this should work with displayEvent too
          return false;
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();
        mitk::StateEvent* newStateEvent = NULL;
        if (this->CheckSelected(worldPoint))
        {
          newStateEvent = new mitk::StateEvent(StYES, posEvent);
          m_DataTreeNode->GetPropertyList()->SetProperty("selected", new mitk::BoolProperty(true));  // TODO: Generate an Select Operation and send it to the undo controller ?
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
    case AcROTATESTART:
    case AcSCALESTART:
      {
        if (posEvent != NULL)
        {     
          mitk::vm2itk(posEvent->GetWorldPosition(), m_LastMousePosition);
          ok = true;
        }
        else if (displayEvent != NULL)  // 2D coordinate in event
        {
          ok = ConvertDisplayEventToWorldPosition(displayEvent, m_LastMousePosition);      
        }
        else
          ok = false;    
        break;
      }
    case AcTRANSLATE:
      {
        mitk::ITKPoint3D newPosition;
        if (posEvent != NULL)   // 3D coordinate in event
        {
          mitk::vm2itk(posEvent->GetWorldPosition(), newPosition);          //converting from Point3D to itk::Point      
          newPosition -=  m_LastMousePosition.GetVectorFromOrigin();        // compute difference between actual and last mouse position
          mitk::vm2itk(posEvent->GetWorldPosition(), m_LastMousePosition);  // save current mouse position as last position
        }
        else if (displayEvent != NULL)  // 2D coordinate in event
        {
          mitk::ITKPoint3D p;
          if(ConvertDisplayEventToWorldPosition(displayEvent, p) == false)
            return false;      
          newPosition = p - m_LastMousePosition.GetVectorFromOrigin();        // compute difference between actual and last mouse position
          m_LastMousePosition = p;    // save current mouse position as last position
        }
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
          OperationEvent *operationEvent = new OperationEvent(geometry, doOp, undoOp, objectEventId, groupEventId);
          m_UndoController->SetOperationEvent(operationEvent);
        }
        /* execute the Operation */
        geometry->ExecuteOperation(doOp);
        ok = true;
        break;
      }
    case AcROTATE:
      {
        mitk::ITKPoint3D p;
        if (posEvent != NULL)   // 3D coordinate in event
        {
          mitk::vm2itk(posEvent->GetWorldPosition(), p);          //converting from Point3D to itk::Point      
        }
        else if (displayEvent != NULL)  // 2D coordinate in event
        {
          if(ConvertDisplayEventToWorldPosition(displayEvent, p) == false)
            return false;
        }
        mitk::ITKVector3D newPosition = p.GetVectorFromOrigin();

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
        m_LastMousePosition = p; // save current mouse position as last mouse position

        /* create operation with center of rotation, angle and axis and send it to the geometry and Undo controller */
        mitk::RotationOperation* doOp = new mitk::RotationOperation(OpROTATE, ITKPoint3D(position) , rotationaxis, angle);

        if (m_UndoEnabled)	//write to UndoMechanism
        {
          RotationOperation* undoOp = new mitk::RotationOperation(OpROTATE, ITKPoint3D(position) , rotationaxis, -angle);
          OperationEvent *operationEvent = new OperationEvent(geometry, doOp, undoOp, objectEventId, groupEventId);
          m_UndoController->SetOperationEvent(operationEvent);
        }
        /* execute the Operation */
        geometry->ExecuteOperation(doOp);
        ok = true;
        break;
      }
    case AcSCALE:
      {    
        mitk::ITKPoint3D p;
        if (posEvent != NULL)   // 3D coordinate in event
        {
          mitk::vm2itk(posEvent->GetWorldPosition(), p);          //converting from Point3D to itk::Point      
        }
        else if (displayEvent != NULL)  // 2D coordinate in event
        {
          if(ConvertDisplayEventToWorldPosition(displayEvent, p) == false)
            return false;
        }
        mitk::ITKVector3D v = p - m_LastMousePosition;    
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
        itk2vtk(p, convert);
        geometry->GetVtkTransform()->GetInverse()->TransformPoint(convert, convert);  // transform end point to local object coordinates
        end[0] = fabs(convert[0]);  end[1] = fabs(convert[1]);  end[2] = fabs(convert[2]);  // mirror it to the positive quadrant

        /* check if mouse movement is towards or away from the objects axes and adjust scale factors accordingly */
        ITKVector3D vLocal = start - end;
        newScale[0] = (vLocal[0] > 0.0) ? -fabs(newScale[0]) : +fabs(newScale[0]);
        newScale[1] = (vLocal[1] > 0.0) ? -fabs(newScale[1]) : +fabs(newScale[1]);
        newScale[2] = (vLocal[2] > 0.0) ? -fabs(newScale[2]) : +fabs(newScale[2]);

        m_LastMousePosition = p;  // update lastPosition for next mouse move

        /* generate Operation and send it to the receiving geometry */
        PointOperation* doOp = new mitk::PointOperation(OpSCALE, newScale, 0); // Index is not used here
        if (m_UndoEnabled)	//write to UndoMechanism
        {
          mitk::ITKPoint3D oldScaleData;
          oldScaleData[0] = -newScale[0];
          oldScaleData[1] = -newScale[1];
          oldScaleData[2] = -newScale[2];

          PointOperation* undoOp = new mitk::PointOperation(OpSCALE, oldScaleData, 0);
          OperationEvent *operationEvent = new OperationEvent(geometry, doOp, undoOp, objectEventId, groupEventId);
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
    selected = box->IsInside(itkPoint); // check if point is inside the datas bounding box
  }
  return selected;
}

bool mitk::AffineInteractor::ConvertDisplayEventToWorldPosition(mitk::DisplayPositionEvent const* displayEvent, mitk::ITKPoint3D& worldPoint)
{
  mitk::Point2D displayPoint = displayEvent->GetDisplayPosition();
  /* Copied from vtk Sphere widget */
  double focalPoint[4], position[4];
  double z;
  mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
  if (globalInteraction == NULL)
    return false;
  const FocusManager::FocusElement* fe = globalInteraction->GetFocus();
  FocusManager::FocusElement* fe2 =  const_cast <FocusManager::FocusElement*>(fe);
  mitk::OpenGLRenderer* glRenderer = dynamic_cast<mitk::OpenGLRenderer*>(fe2);
  if (glRenderer == NULL)
    return false;
  vtkRenderer *renderer = glRenderer->GetVtkRenderer();
  vtkCamera *camera = renderer->GetActiveCamera();
  if ( !camera )
  {
    return false;
  }
  // Compute the two points defining the motion vector
  camera->GetFocalPoint(focalPoint);
  //this->ComputeWorldToDisplay(focalPoint[0], focalPoint[1], focalPoint[2], focalPoint);
  renderer->SetWorldPoint(focalPoint[0], focalPoint[1], focalPoint[2], 1.0);
  renderer->WorldToDisplay();
  renderer->GetDisplayPoint(focalPoint);
  z = focalPoint[2];
  //      this->ComputeDisplayToWorld(displayPoint.x, displayPoint.y, z, position);
  renderer->SetDisplayPoint(displayPoint.x, displayPoint.y, z);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(position);
  if (position[3])
  {
    worldPoint[0] = position[0] / position[3];
    worldPoint[1] = position[1] / position[3];
    worldPoint[2] = position[2] / position[3];
    position[3] = 1.0;
  }
  return true;
}
