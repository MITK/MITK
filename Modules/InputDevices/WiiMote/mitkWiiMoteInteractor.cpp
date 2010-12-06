#include <mitkWiiMoteInteractor.h>

// mitk
#include <mitkInteractionConst.h>
#include <mitkWiiMoteAllDataEvent.h>
#include <mitkVector.h>
#include <mitkSurface.h>
#include <mitkGeometry3D.h>
#include <mitkRenderingManager.h>
#include <mitkVector.h>
#include <mitkBaseRenderer.h>
#include <mitkGlobalInteraction.h>

// vtk
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkTransform.h>

#define _USE_MATH_DEFINES // otherwise, constants will not work
#include <math.h>



mitk::WiiMoteInteractor::WiiMoteInteractor(const char* type, DataNode* dataNode)
: Interactor(type, dataNode)
, m_OrientationX(0)
, m_OrientationY(0)
, m_OrientationZ(0)
, m_xVelocity (0)
, m_yVelocity (0)
, m_zVelocity (0)
, m_xAngle (0)
, m_yAngle (0)
, m_zAngle (0)
, m_InRotation(false)
{
  CONNECT_ACTION(mitk::AcONWIIMOTEINPUT,OnWiiMoteInput);
  CONNECT_ACTION(mitk::AcONWIIMOTEBUTTONRELEASED,OnWiiMoteReleaseButton);
}

mitk::WiiMoteInteractor::~WiiMoteInteractor()
{

}

bool mitk::WiiMoteInteractor::OnWiiMoteActivateButton(Action* action, const mitk::StateEvent* stateEvent)
{

  return true;
}

bool mitk::WiiMoteInteractor::OnWiiMoteInput(Action* action, const mitk::StateEvent* stateEvent)
{
  const mitk::WiiMoteAllDataEvent* wiiMoteEvent; 
  try
  {
    wiiMoteEvent = dynamic_cast<const mitk::WiiMoteAllDataEvent*>(stateEvent->GetEvent());
  }
  catch(...)
  {
    MITK_ERROR << "Event is not wiimote event and could not be transformed\n";
  }

  //checking corresponding Data; has to be a surface or a subclass
  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_DataNode->GetData());
  if ( surface == NULL )
  {
    MITK_WARN<<"Wiimote Interactor got wrong type of data! Aborting interaction!\n";
    return false;
  }
  Geometry3D* geometry = surface->GetUpdatedTimeSlicedGeometry()->GetGeometry3D( m_TimeStep );

  m_OrientationX = wiiMoteEvent->GetOrientationX();
  m_OrientationY = wiiMoteEvent->GetOrientationY();
  m_OrientationZ = wiiMoteEvent->GetOrientationZ();
  
  ScalarType pitchSpeed = wiiMoteEvent->GetPitchSpeed();
  ScalarType rollSpeed = wiiMoteEvent->GetRollSpeed();
  ScalarType yawSpeed = wiiMoteEvent->GetYawSpeed();

  // angle x
  if(std::abs(pitchSpeed) < 200)
    pitchSpeed = 0;

  m_xAngle += (pitchSpeed / 1500);

  // angle y
  if(std::abs(rollSpeed) < 200)
    rollSpeed = 0;

  m_yAngle += (rollSpeed / 1500);

  // angle z 
  if(std::abs(yawSpeed) < 200)
    yawSpeed = 0;

  m_zAngle += (yawSpeed / 1500);

  if(std::abs(pitchSpeed) > 200 
    || std::abs(rollSpeed) > 200 
    || std::abs(yawSpeed) > 200)
  {
    m_InRotation = true;
    ////// depending on a combination of the
    ////// orientation the angleX wil be altered
    ////// because the range from roll is limited
    ////// range: -90° to 90° by the wiimote 
    ////if(wiiMoteEvent->GetOrientationZ() < 0)
    ////{
    ////  // value is positive 
    ////  if(wiiMoteEvent->GetOrientationX() > 0)
    ////  {
    ////    // the degree measured decreases after it reaches 
    ////    // in the "real" world the 90 degree angle 
    ////    // (rotation to the right side)
    ////    // therefore it needs to artificially increased

    ////    // measured value drops -> computated angle increases
    ////    angleX = 90 - angleX;

    ////    // now add the "new" angle to 90 degree threshold
    ////    angleX += 90;
    ////  }
    ////  // value is negative
    ////  else if(wiiMoteEvent->GetOrientationX() < 0)
    ////  {
    ////    // the degree measured increases after it reaches
    ////    // in the "real" world -90 degree 
    ////    // (rotation to the left side)
    ////    // therefore it needs to be artificially decreased 
    ////    // (example -90 -> -70, but -110 is needed)

    ////    // measured value increases -> computated angle decreases
    ////    angleX = 90 + angleX;

    ////    // invert the algebraic sign, because it is the "negative"
    ////    // side of the rotation
    ////    angleX = -angleX;

    ////    // now add the negative value to the -90 degree threshold
    ////    // to decrease the value further
    ////    angleX -= 90;
    ////  }
    ////  else if(wiiMoteEvent->GetOrientationX() == 0)
    ////  {
    ////    // i.e. wiimote is flipped upside down
    ////    angleX = 180;
    ////  }
    ////}

    //rotation
    vtkTransform *vtkTransform = vtkTransform::New();

    //copy m_vtkMatrix to m_VtkIndexToWorldTransform
    geometry->TransferItkToVtkTransform();

    //m_VtkIndexToWorldTransform as vtkLinearTransform*
    vtkTransform->SetMatrix(geometry->GetVtkTransform()->GetMatrix());

    // rotation from center is different
    // from rotation while translated
    // hence one needs the center of the object
    Point3D center = geometry->GetOrigin();

    vtkTransform->PostMultiply();
    vtkTransform->Translate(-center[0], -center[1], -center[2]);
    //vtkTransform->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);

    vtkTransform->RotateX(m_xAngle);

    vtkTransform->RotateY(m_zAngle);

    vtkTransform->RotateZ(m_yAngle);

    vtkTransform->Translate(center[0], center[1], center[2]);
    vtkTransform->PreMultiply();

    geometry->SetIndexToWorldTransformByVtkMatrix(vtkTransform->GetMatrix());
    geometry->Modified();

    // indicate modification of data tree node
    m_DataNode->Modified();

    vtkTransform->Delete();

    //update rendering
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    return true;

  }
  else if(!m_InRotation)
  {
    float xValue = wiiMoteEvent->GetXAcceleration();
    float yValue = wiiMoteEvent->GetYAcceleration();
    float zValue = wiiMoteEvent->GetZAcceleration();

    float pitch = wiiMoteEvent->GetPitch();
    float roll = wiiMoteEvent->GetRoll();

    // substracts the proportionate force 
    // applied by gravity depending on the
    // orientation

    float sinP = sin(pitch/180.0 * M_PI);
    float cosP = cos(pitch/180.0 * M_PI);
    float sinR = sin(roll/180.0 * M_PI);
    float cosR = cos(roll/180.0 * M_PI);

    // x acceleration
    if(m_OrientationZ >= 0)
      xValue = xValue - sinR * cosP;
    else
      xValue = xValue + sinR * cosP;

    // against drift
    if(std::abs(xValue) < 0.3)
      xValue = 0;

    // y acceleration
    yValue = yValue + sinP;

    // against drift
    if(std::abs(yValue) < 0.3)
      yValue = 0;

    // z acceleration
    zValue = zValue - cosP * cosR;

    // against drift
    if(std::abs(zValue) < 0.3)
      zValue = 0;

    // simple integration over time
    // resulting in velocity  
    m_xVelocity -= xValue;
    m_yVelocity -= yValue;
    m_zVelocity += zValue;

    // translation
    mitk::Vector3D movementVector;
    movementVector.SetElement(0,m_xVelocity);
    movementVector.SetElement(1,m_yVelocity);
    movementVector.SetElement(2,m_zVelocity);

    geometry->Translate(movementVector);

    // indicate modification of data tree node
    m_DataNode->Modified();

    // update rendering
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    return true;
  }

  return false;
}

bool mitk::WiiMoteInteractor::OnWiiMoteReleaseButton(Action* action, const mitk::StateEvent* stateEvent)
{
  m_xVelocity = 0;
  m_yVelocity = 0;
  m_zVelocity = 0;

  m_xAngle = 0;
  m_yAngle = 0;
  m_zAngle = 0;

  m_InRotation = false;

  return true;
}