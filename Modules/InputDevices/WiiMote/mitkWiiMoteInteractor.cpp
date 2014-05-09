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
#include <mitkWiiMoteInteractor.h>

// mitk
#include <mitkInteractionConst.h>
#include <mitkNumericTypes.h>
#include <mitkSurface.h>
#include <mitkRenderingManager.h>
#include <mitkNumericTypes.h>
#include <mitkGlobalInteraction.h>
#include <mitkDataStorage.h>
#include <mitkRenderingManager.h>
#include <mitkVtkPropRenderer.h>

// vtk
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkTransform.h>
#include <vtkRenderer.h>
#include <vtkMath.h>

// vnl
#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_inverse.h>

#define _USE_MATH_DEFINES // otherwise, constants will not work
#include <math.h>

const double DELTATIME = 0.01;

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
, m_xValue (0)
, m_yValue (0)
, m_zValue (0)
, m_InRotation(false)
, m_TranslationMode(1)
, m_OriginalGeometry(NULL)
, m_SurfaceInteractionMode(1)
{
  // save original geometry
  mitk::Geometry3D* temp = this->TransformCurrentDataInGeometry3D();
  try
  {
    m_OriginalGeometry = dynamic_cast<mitk::Geometry3D*>(temp->Clone().GetPointer());
  }
  catch(...)
  {
    MITK_WARN << "Original geometry could not be stored!";
  }

  // connect actions to methods
  CONNECT_ACTION(mitk::AcONWIIMOTEINPUT,OnWiiMoteInput);
  CONNECT_ACTION(mitk::AcONWIIMOTEBUTTONRELEASED,OnWiiMoteReleaseButton);
  CONNECT_ACTION(mitk::AcRESETVIEW,OnWiiMoteResetButton);
}

mitk::WiiMoteInteractor::~WiiMoteInteractor()
{

}

bool mitk::WiiMoteInteractor::OnWiiMoteResetButton(Action* action, const mitk::StateEvent* stateEvent)
{
  // resets the geometry, so that the
  // object will be returned to its
  // initial state
  try
  {
    mitk::Surface* surface
      = dynamic_cast<mitk::Surface*>(m_DataNode->GetData());
    mitk::Geometry3D::Pointer temp
      = dynamic_cast<mitk::Geometry3D*>(m_OriginalGeometry->Clone().GetPointer());
    surface->SetGeometry(temp);

    if(surface == NULL)
    {
      MITK_WARN << "Original geometry could not be used for reset!";
    }

    m_DataNode->SetData(surface);

    m_DataNode->Modified();
  }
  catch(...)
  {
    MITK_ERROR << "Original geometry could not be retrieved";
  }

  //reset the camera, so that the objects shown in the scene can be seen.
  const mitk::BaseRenderer* br = mitk::GlobalInteraction::GetInstance()->GetFocus();

  const mitk::VtkPropRenderer* glRenderer
    = dynamic_cast<const mitk::VtkPropRenderer*>(br);
  if (glRenderer)
  {
    vtkRenderer* vtkRenderer = glRenderer->GetVtkRenderer();
    mitk::DataStorage* ds = br->GetDataStorage();
    if (ds == NULL)
      return false;

    mitk::BoundingBox::Pointer bb = ds->ComputeBoundingBox();

    mitk::Point3D middle = bb->GetCenter();
    vtkRenderer->GetActiveCamera()->SetFocalPoint(middle[0],middle[1],middle[2]);

    vtkRenderer->ResetCamera();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return true;
  }

  return false;
}

bool mitk::WiiMoteInteractor::OnWiiMoteInput(Action* action, const mitk::StateEvent* stateEvent)
{
  const mitk::WiiMoteAllDataEvent* wiiMoteEvent;
  try
  {
    wiiMoteEvent =
      dynamic_cast<const mitk::WiiMoteAllDataEvent*>(stateEvent->GetEvent());
  }
  catch(...)
  {
    MITK_ERROR << "Event is not wiimote event and could not be transformed\n";
  }

  m_SurfaceInteractionMode = wiiMoteEvent->GetSurfaceInteractionMode();

  //this->FixedRotationAndTranslation(wiiMoteEvent);

  // -------------------- values for translation --------------------
  float xAccel = wiiMoteEvent->GetXAcceleration();
  float yAccel = wiiMoteEvent->GetYAcceleration();
  float zAccel = wiiMoteEvent->GetZAcceleration();

  float pitch = wiiMoteEvent->GetPitch();
  float roll = wiiMoteEvent->GetRoll();

  m_OrientationX = wiiMoteEvent->GetOrientationX();
  m_OrientationY = wiiMoteEvent->GetOrientationY();
  m_OrientationZ = wiiMoteEvent->GetOrientationZ();

  // substracts the proportionate force
  // applied by gravity depending on the
  // orientation

  float sinP = sin(pitch/180.0 * M_PI);
  float cosP = cos(pitch/180.0 * M_PI);
  float sinR = sin(roll/180.0 * M_PI);
  float cosR = cos(roll/180.0 * M_PI);

  // x acceleration
  if(m_OrientationZ >= 0)
  {
    m_xValue = xAccel - sinR * cosP;
  }
  else
  {
    m_xValue = xAccel + sinR * cosP;
  }

  //// against drift
  //if(std::abs(xAccel) < 0.2)
  //{
  //  m_xValue = 0;
  //}

  // y acceleration
  m_yValue = yAccel + sinP;

  //// against drift
  //if(std::abs(yAccel) < 0.2)
  //{
  //  m_yValue = 0;
  //}

  // z acceleration
  m_zValue = zAccel - cosP * cosR;

  //// against drift
  //if(std::abs(zAccel) < 0.3)
  //{
  //  m_zValue = 0;
  //}

  m_xVelocity += m_xValue;
  m_yVelocity += m_yValue;
  m_zVelocity -= m_zValue;

  // -------------------- values for rotation --------------------

  ScalarType pitchSpeed = wiiMoteEvent->GetPitchSpeed();
  ScalarType rollSpeed = wiiMoteEvent->GetRollSpeed();
  ScalarType yawSpeed = wiiMoteEvent->GetYawSpeed();

  // x angle

  if(std::abs(pitchSpeed) > 50
    && std::abs(pitchSpeed) < 1000)
  {
    if(m_SurfaceInteractionMode == 1)
    {
      m_xAngle = (pitchSpeed * DELTATIME);
    }
    else
    {
      m_xAngle = (-pitchSpeed * DELTATIME);
    }
  }
  else
  {
    m_xAngle = 0;
  }

  // y angle

  if(std::abs(rollSpeed) > 50
    && std::abs(rollSpeed) < 1000)
  {
    m_yAngle = (rollSpeed * DELTATIME);
  }
  else
  {
    m_yAngle = 0;
  }

  // z angle

  if(std::abs(yawSpeed) > 50
    && std::abs(yawSpeed) < 1000)
  {
    if(m_SurfaceInteractionMode == 1)
    {
      m_zAngle = (yawSpeed * DELTATIME);
    }
    else
    {
      m_zAngle = (-yawSpeed * DELTATIME);
    }
  }
  else
  {
    m_zAngle = 0;
  }

  // -------------------- rotation and translation --------------------

  bool result = false;

  result = this->DynamicRotationAndTranslation(this->TransformCurrentDataInGeometry3D());

  return result;
}

bool mitk::WiiMoteInteractor::OnWiiMoteReleaseButton(Action* action, const mitk::StateEvent* stateEvent)
{
  m_xVelocity = 0;
  m_yVelocity = 0;
  m_zVelocity = 0;

  m_xValue = 0;
  m_yValue = 0;
  m_zValue = 0;

  m_xAngle = 0;
  m_yAngle = 0;
  m_zAngle = 0;

  // only for fixed translation
  m_InRotation = false;
  m_TranslationMode = 1;

  return true;
}

mitk::Geometry3D* mitk::WiiMoteInteractor::TransformCurrentDataInGeometry3D()
{
  //checking corresponding Data; has to be a surface or a subclass
  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_DataNode->GetData());
  if ( surface == NULL )
  {
    MITK_WARN<<"Wiimote Interactor got wrong type of data! Aborting interaction!\n";
    return NULL;
  }
  Geometry3D* geometry = surface->GetUpdatedTimeGeometry()->GetGeometryForTimeStep( m_TimeStep );
  return geometry;
}

vnl_matrix_fixed<double, 4, 4> mitk::WiiMoteInteractor::ComputeCurrentCameraPosition( vtkCamera* vtkCamera )
{
  vnl_matrix_fixed<double, 4, 4> cameraMat;

   //first we need the position of the camera
  mitk::Vector3D camPosition;
  double camPositionTemp[3];
  vtkCamera->GetPosition(camPositionTemp);
  camPosition[0] = camPositionTemp[0];
  camPosition[1] = camPositionTemp[1];
  camPosition[2] = camPositionTemp[2];

  //then the upvector of the camera
  mitk::Vector3D upCamVector;
  double upCamTemp[3];
  vtkCamera->GetViewUp(upCamTemp);
  upCamVector[0] = upCamTemp[0];
  upCamVector[1] = upCamTemp[1];
  upCamVector[2] = upCamTemp[2];
  upCamVector.Normalize();

  //then the vector to which the camera is heading at (focalpoint)
  mitk::Vector3D focalPoint;
  double focalPointTemp[3];
  vtkCamera->GetFocalPoint(focalPointTemp);
  focalPoint[0] = focalPointTemp[0];
  focalPoint[1] = focalPointTemp[1];
  focalPoint[2] = focalPointTemp[2];
  mitk::Vector3D focalVector;
  focalVector = focalPoint - camPosition;
  focalVector.Normalize();

  //orthogonal vector to focalVector and upCamVector
  mitk::Vector3D crossVector;
  crossVector = CrossProduct(upCamVector, focalVector);
  crossVector.Normalize();

  cameraMat.put(0,0,crossVector[0]);
  cameraMat.put(1,0,crossVector[1]);
  cameraMat.put(2,0,crossVector[2]);
  cameraMat.put(3,0,0);

  cameraMat.put(0,1,focalVector[0]);
  cameraMat.put(1,1,focalVector[1]);
  cameraMat.put(2,1,focalVector[2]);
  cameraMat.put(3,1,0);

  cameraMat.put(0,2,upCamVector[0]);
  cameraMat.put(1,2,upCamVector[1]);
  cameraMat.put(2,2,upCamVector[2]);
  cameraMat.put(3,2,0);

  cameraMat.put(0,3,camPosition[0]);
  cameraMat.put(1,3,camPosition[1]);
  cameraMat.put(2,3,camPosition[2]);
  cameraMat.put(3,3,1);

  return cameraMat;
}

bool mitk::WiiMoteInteractor::DynamicRotationAndTranslation(Geometry3D* geometry)
{
   // computation of the delta transformation
  if(m_SurfaceInteractionMode == 1)
  {
    // necessary because the wiimote has
    // a different orientation when loaded
    // as an object file
    ScalarType temp = m_yAngle;
    m_yAngle = m_zAngle;
    m_zAngle = temp;
  }

  //vnl_quaternion<double> Rx(m_OrientationX
  //  ,m_OrientationY
  //  ,m_OrientationZ
  //  , m_xAngle);

  //vnl_quaternion<double> Ry(Rx.axis()[0]
  //  , Rx.axis()[1]
  //  , Rx.axis()[2]
  //  , m_yAngle);

  //vnl_quaternion<double> Rz(Ry.axis()[0]
  //  , Ry.axis()[1]
  //  , Ry.axis()[2]
  //  , m_zAngle);


  vnl_quaternion<double> q(
    vtkMath::RadiansFromDegrees( m_xAngle ),
    vtkMath::RadiansFromDegrees( m_yAngle ),
    vtkMath::RadiansFromDegrees( m_zAngle ) );

  //q = Rz * Ry * Rx;
  //q.normalize();

  vnl_matrix_fixed<double, 4, 4> deltaTransformMat = q.rotation_matrix_transpose_4();

  // fill translation column
  deltaTransformMat(0,3) = m_xVelocity;
  deltaTransformMat(1,3) = m_yVelocity;
  deltaTransformMat(2,3) = m_zVelocity;

  // invert matrix to apply
  // correct order for the transformation
  deltaTransformMat = vnl_inverse(deltaTransformMat);

  vtkMatrix4x4* deltaTransform = vtkMatrix4x4::New();

  // copy into matrix
  for(size_t i=0; i<4; ++i)
    for(size_t j=0; j<4; ++j)
      deltaTransform->SetElement(i,j, deltaTransformMat(i,j));

  vtkMatrix4x4* objectTransform = vtkMatrix4x4::New();

  if(m_SurfaceInteractionMode == 2)
  {
    // additional computation for transformation
    // relative to the camera view

    // get renderer
    const RenderingManager::RenderWindowVector& renderWindows
      = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
    for ( RenderingManager::RenderWindowVector::const_iterator iter = renderWindows.begin();
      iter != renderWindows.end();
      ++iter )
    {
      if ( mitk::BaseRenderer::GetInstance((*iter))->GetMapperID() == BaseRenderer::Standard3D )
      {
        m_BaseRenderer = mitk::BaseRenderer::GetInstance((*iter));
      }
    }

    vtkCamera* camera
      = m_BaseRenderer->GetVtkRenderer()->GetActiveCamera();

    //vtkMatrix4x4* cameraMat = vtkMatrix4x4::New();
    vnl_matrix_fixed<double, 4, 4> cameraMat;
    vnl_matrix_fixed<double, 4, 4> objectMat;

    // copy object matrix
    for(size_t i=0; i<4; ++i)
      for(size_t j=0; j<4; ++j)
        objectMat.put(i,j, geometry->GetVtkTransform()->GetMatrix()->GetElement(i,j));

    cameraMat = this->ComputeCurrentCameraPosition(camera);

    vnl_matrix_fixed<double, 4, 4> newObjectMat;
    vnl_matrix_fixed<double, 4, 4> objectToCameraMat;

    objectToCameraMat = vnl_inverse(cameraMat) * objectMat;

    newObjectMat = vnl_inverse(objectToCameraMat)
      * deltaTransformMat
      * objectToCameraMat
      * vnl_inverse(objectMat);

    newObjectMat = vnl_inverse(newObjectMat);
    newObjectMat.put(0,3,objectMat(0,3)+deltaTransformMat(0,3));
    newObjectMat.put(1,3,objectMat(1,3)+deltaTransformMat(1,3));
    newObjectMat.put(2,3,objectMat(2,3)+deltaTransformMat(2,3));


    // copy result
    for(size_t i=0; i<4; ++i)
      for(size_t j=0; j<4; ++j)
        objectTransform->SetElement(i,j, newObjectMat(i,j));
  }

  //copy m_vtkMatrix to m_VtkIndexToWorldTransform
  geometry->TransferItkToVtkTransform();

  vtkTransform* vtkTransform = vtkTransform::New();

  if(m_SurfaceInteractionMode == 1)
  {
    //m_VtkIndexToWorldTransform as vtkLinearTransform*
    vtkTransform->SetMatrix( geometry->GetVtkTransform()->GetMatrix() );

    vtkTransform->Concatenate( deltaTransform );

    geometry->SetIndexToWorldTransformByVtkMatrix( vtkTransform->GetMatrix() );

  }
  else
  {
    geometry->SetIndexToWorldTransformByVtkMatrix( objectTransform );
  }

  geometry->Modified();
  m_DataNode->Modified();

  vtkTransform->Delete();
  objectTransform->Delete();

  deltaTransform->Delete();

  //update rendering
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return true;
}


bool mitk::WiiMoteInteractor::FixedRotationAndTranslation(const mitk::WiiMoteAllDataEvent* wiiMoteEvent)
{

  Geometry3D* geometry = this->TransformCurrentDataInGeometry3D();

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

  if(  std::abs(pitchSpeed) > 200
    || std::abs(rollSpeed) > 200
    || std::abs(yawSpeed) > 200)
  {
    m_InRotation = true;
    //// depending on a combination of the
    //// orientation the angleX wil be altered
    //// because the range from roll is limited
    //// range: -90� to 90� by the wiimote
    //if(wiiMoteEvent->GetOrientationZ() < 0)
    //{
    //  // value is positive
    //  if(wiiMoteEvent->GetOrientationX() > 0)
    //  {
    //    // the degree measured decreases after it reaches
    //    // in the "real" world the 90 degree angle
    //    // (rotation to the right side)
    //    // therefore it needs to artificially increased

    //    // measured value drops -> computated angle increases
    //    angleX = 90 - angleX;

    //    // now add the "new" angle to 90 degree threshold
    //    angleX += 90;
    //  }
    //  // value is negative
    //  else if(wiiMoteEvent->GetOrientationX() < 0)
    //  {
    //    // the degree measured increases after it reaches
    //    // in the "real" world -90 degree
    //    // (rotation to the left side)
    //    // therefore it needs to be artificially decreased
    //    // (example -90 -> -70, but -110 is needed)

    //    // measured value increases -> computated angle decreases
    //    angleX = 90 + angleX;

    //    // invert the algebraic sign, because it is the "negative"
    //    // side of the rotation
    //    angleX = -angleX;

    //    // now add the negative value to the -90 degree threshold
    //    // to decrease the value further
    //    angleX -= 90;
    //  }
    //  else if(wiiMoteEvent->GetOrientationX() == 0)
    //  {
    //    // i.e. wiimote is flipped upside down
    //    angleX = 180;
    //  }
    //}

    //rotation
    vtkTransform *vtkTransform = vtkTransform::New();

    //copy m_vtkMatrix to m_VtkIndexToWorldTransform
    geometry->TransferItkToVtkTransform();

    //////m_VtkIndexToWorldTransform as vtkLinearTransform*
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
    if(std::abs(xValue) < 0.2)
      xValue = 0;

    // y acceleration
    yValue = yValue + sinP;

    // against drift
    if(std::abs(yValue) < 0.2)
      yValue = 0;

    // z acceleration
    zValue = zValue - cosP * cosR;

    // against drift
    if(std::abs(zValue) < 0.3)
      zValue = 0;

    // simple integration over time
    // resulting in velocity
    switch(m_TranslationMode)
    {
    case 1:
      m_xVelocity -= xValue;
      m_yVelocity -= yValue;
      m_zVelocity += zValue;


      // 1 = movement to the right
      // initially starts with negative acceleration
      // 2 = movement to the left
      // initially starts with positive acceleration
      if(  m_xVelocity > 0 && xValue > 0  // 1
        || m_xVelocity < 0 && xValue < 0) // 2
      {
        m_xVelocity += xValue;
      }
      else if( m_xVelocity > 0 && xValue < 0  // 1
        || m_xVelocity < 0 && xValue > 0)     // 2
      {
        m_xVelocity -= xValue;
      }
      break;

    case 3:

      m_yVelocity -= yValue;
      break;
    case 4:

      // 1 = movement up
      // initially starts with positive acceleration
      // 2 = movement down
      // initially starts with negative acceleration
      if(  m_zVelocity > 0 && zValue < 0  // 1
        || m_zVelocity < 0 && zValue > 0) // 2
      {
        m_zVelocity -= zValue;
      }
      else if(m_zVelocity > 0 && zValue > 0 // 1
        || m_zVelocity < 0 && zValue < 0)   // 2
      {
        m_zVelocity += zValue;
      }
      break;
    }

    // sets the mode of the translation
    // depending on the initial velocity
    if(  std::abs(m_xVelocity) > std::abs(m_yVelocity)
      && std::abs(m_xVelocity) > std::abs(m_zVelocity) )
    {
      m_TranslationMode = 2;
      m_yVelocity = 0;
      m_zVelocity = 0;
    }
    else if( std::abs(m_yVelocity) > std::abs(m_xVelocity)
      && std::abs(m_yVelocity) > std::abs(m_zVelocity) )
    {
      m_TranslationMode = 3;
      m_xVelocity = 0;
      m_zVelocity = 0;
    }
    else if(std::abs(m_zVelocity) > std::abs(m_xVelocity)
      && std::abs(m_zVelocity) > std::abs(m_yVelocity) )
    {
      m_TranslationMode = 4;
      m_xVelocity = 0;
      m_yVelocity = 0;
    }

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
