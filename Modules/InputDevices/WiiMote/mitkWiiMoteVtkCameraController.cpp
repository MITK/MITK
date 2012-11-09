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
#include "mitkWiiMoteVtkCameraController.h"

#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "mitkStateEvent.h"
#include "mitkSurface.h"

#include "mitkVtkPropRenderer.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"

// factor derived from the asumption that
// the field of view in the virtual reality
// is limited to an angle of 45°
const double CALIBRATIONFACTORY = 0.125;
const double CALIBRATIONFACTORX = 0.125;

// max resolution of the cam 1024x768
const double XMIN = 0;
const double XMAX = 1024;
const double YMIN = 0;
const double YMAX = 768;

// initial scroll value

const int UPDATEFREQUENCY = 5;

mitk::WiiMoteVtkCameraController::WiiMoteVtkCameraController()
: CameraController("WiiMoteHeadtracking")
, m_ClippingRangeIsSet(false)
, m_SensitivityXMIN (XMAX)
, m_SensitivityXMAX (XMIN)
, m_SensitivityYMIN (YMAX)
, m_SensitivityYMAX (YMIN)
, m_SensitivityX (0)
, m_SensitivityY (0)
, m_Calibrated (false)
, m_AxialBR( NULL )
, m_InitialScrollValue( 0 )
, m_UpdateFrequency( 0 )
, m_CurrentElevationAngle ( 0 )
, m_CurrentAzimuthAngle ( 0 )
{
  CONNECT_ACTION(mitk::AcONWIIMOTEINPUT, OnWiiMoteInput);
  CONNECT_ACTION(mitk::AcRESETVIEW, ResetView);
  CONNECT_ACTION(mitk::AC_INIT, InitCalibration);
  CONNECT_ACTION(mitk::AcCHECKPOINT, Calibration);
  CONNECT_ACTION(mitk::AcFINISH, FinishCalibration);
}

mitk::WiiMoteVtkCameraController::~WiiMoteVtkCameraController()
{
}

bool mitk::WiiMoteVtkCameraController::OnWiiMoteInput(mitk::Action* a, const mitk::StateEvent* e)
{
  //only if 3D rendering
  const mitk::BaseRenderer* br = mitk::GlobalInteraction::GetInstance()->GetFocus();
  this->SetRenderer( br );
  mitk::BaseRenderer::MapperSlotId id = ((mitk::BaseRenderer*)(br))->GetMapperID();
  if (id != mitk::BaseRenderer::Standard3D)
    return true;

  //only if focused by the FocusManager
  if (this->GetRenderer() != mitk::GlobalInteraction::GetInstance()->GetFocus())
    return true;

  //pre-checking for safety
  vtkRenderer* vtkRenderer = ((mitk::VtkPropRenderer*)this->GetRenderer())->GetVtkRenderer();
  if (vtkRenderer == NULL)
    return false;

  vtkCamera* vtkCam = (vtkCamera*)vtkRenderer->GetActiveCamera();

  //TODO check the range
  if(!m_ClippingRangeIsSet)
    vtkCam->SetClippingRange(0.1, 1000);

  const mitk::WiiMoteIREvent* wiiMoteIREvent;

  if(!(wiiMoteIREvent = dynamic_cast<const mitk::WiiMoteIREvent*>(e->GetEvent())))
  {
    MITK_ERROR << "Not a WiiMote Event!";
    return false;
  }

  // get data from the Wiimote
  mitk::Vector2D tempMovementVector(wiiMoteIREvent->GetMovementVector());
  float inputCoordinates[3] = {tempMovementVector[0],tempMovementVector[1], 0};
  mitk::Vector3D movementVector(inputCoordinates);

  //compute current sensitivity according to current BoundingBox of the whole scene!
  double sceneSensivity = 1.0;

  mitk::DataStorage* ds = m_Renderer->GetDataStorage();
  mitk::BoundingBox::Pointer bb = ds->ComputeBoundingBox();
  mitk::BoundingBox::AccumulateType length = bb->GetDiagonalLength2();
  if (length > 0.00001)//if length not zero
    sceneSensivity *= 100.0 / (sqrt(length)) ;

  //sensivity to adapt to mitk speed
  movementVector *= sceneSensivity;

  if(!m_Calibrated)
  {
    movementVector[0] *= CALIBRATIONFACTORX;
    movementVector[1] *= CALIBRATIONFACTORY;
  }
  else
  {
    movementVector[0] *= m_SensitivityX;
    movementVector[1] *= m_SensitivityY;
  }

  // inverts y direction to simulate a 3D View
  // x direction is already inverted
  movementVector[1] *= -1;

  m_CurrentElevationAngle += movementVector[1];

  MITK_INFO << "Elevation angle: " << m_CurrentElevationAngle;

  // avoids flipping of the surface
  // through the elevation function
  if(m_CurrentElevationAngle < 70
    && m_CurrentElevationAngle > -70)
  {
    vtkCam->Elevation((double)movementVector[1]);
  }
  else if( m_CurrentElevationAngle > 70 )
  {
    m_CurrentElevationAngle = 70;
  }
  else if( m_CurrentElevationAngle < -70 )
  {
    m_CurrentElevationAngle = -70;
  }

  m_CurrentAzimuthAngle += movementVector[0];

  // avoids spinning of the surface
  if(m_CurrentAzimuthAngle < 70
    && m_CurrentAzimuthAngle > -70)
  {
    vtkCam->Azimuth((double)movementVector[0]);
  }
  else if( m_CurrentAzimuthAngle > 70 )
  {
    m_CurrentAzimuthAngle = 70;
  }
  else if( m_CurrentAzimuthAngle < -70 )
  {
    m_CurrentAzimuthAngle = -70;
  }


  ////compute the global space coordinates from the relative mouse coordinate
  ////first we need the position of the camera
  //mitk::Vector3D camPosition;
  //double camPositionTemp[3];
  //vtkCam->GetPosition(camPositionTemp);
  //camPosition[0] = camPositionTemp[0]; camPosition[1] = camPositionTemp[1]; camPosition[2] = camPositionTemp[2];

  ////then the upvector of the camera
  //mitk::Vector3D upCamVector;
  //double upCamTemp[3];
  //vtkCam->GetViewUp(upCamTemp);
  //upCamVector[0] = upCamTemp[0]; upCamVector[1] = upCamTemp[1]; upCamVector[2] = upCamTemp[2];
  //upCamVector.Normalize();

  ////then the vector to which the camera is heading at (focalpoint)
  //mitk::Vector3D focalPoint;
  //double focalPointTemp[3];
  //vtkCam->GetFocalPoint(focalPointTemp);
  //focalPoint[0] = focalPointTemp[0]; focalPoint[1] = focalPointTemp[1]; focalPoint[2] = focalPointTemp[2];
  //mitk::Vector3D focalVector;
  //focalVector = focalPoint - camPosition;
  //focalVector.Normalize();

  ////orthogonal vector to focalVector and upCamVector
  //mitk::Vector3D crossVector;
  //crossVector = CrossProduct(upCamVector, focalVector);
  //crossVector.Normalize();

  ////now we have the current orientation so we can adapt it according to the current information, which we got from the Wiimote

  ////new position of the camera:
  ////left/right = camPosition + crossVector * movementVector[0];
  //mitk::Vector3D vectorX = crossVector * -movementVector[0]; //changes the magnitude, not the direction
  //double nextCamPosition[3];
  //nextCamPosition[0] = camPosition[0] + vectorX[0];
  //nextCamPosition[1] = camPosition[1] + vectorX[1];
  //nextCamPosition[2] = camPosition[2] + vectorX[2];

  ////now the up/down movement
  //mitk::Vector3D vectorY = upCamVector * movementVector[1]; //changes the magnitude, not the direction
  //nextCamPosition[0] += vectorY[0];
  //nextCamPosition[1] += vectorY[1];
  //nextCamPosition[2] += vectorY[2];

  ////forward/backward movement
  //mitk::Vector3D vectorZ = focalVector * -movementVector[2]; //changes the magnitude, not the direction
  //nextCamPosition[0] += vectorZ[0];
  //nextCamPosition[1] += vectorZ[1];
  //nextCamPosition[2] += vectorZ[2];

  ////set the next position
  //double nextPosition[3];
  //nextPosition[0] = nextCamPosition[0]; nextPosition[1] = nextCamPosition[1]; nextPosition[2] = nextCamPosition[2];
  //vtkCam->SetPosition(nextPosition);

  ////adapt the focal point the same way
  //double currentFocalPoint[3], nextFocalPoint[3];
  //vtkCam->GetFocalPoint(currentFocalPoint);
  //nextFocalPoint[0] = currentFocalPoint[0] + vectorX[0] + vectorY[0] + vectorZ[0];
  //nextFocalPoint[1] = currentFocalPoint[1] + vectorX[1] + vectorY[1] + vectorZ[1]; ;
  //nextFocalPoint[2] = currentFocalPoint[2] + vectorX[2] + vectorY[2] + vectorZ[2];
  //vtkCam->SetFocalPoint(nextFocalPoint);

  //Reset the camera clipping range based on the bounds of the visible actors.
  //This ensures that no props are cut off
  vtkRenderer->ResetCameraClippingRange();

  // ------------ axial scrolling -----------------------

  // get renderer
  const RenderingManager::RenderWindowVector& renderWindows
    = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  for ( RenderingManager::RenderWindowVector::const_iterator iter = renderWindows.begin();
    iter != renderWindows.end();
    ++iter )
  {
    if ( mitk::BaseRenderer::GetInstance((*iter))->GetMapperID() == BaseRenderer::Standard2D )
    {
      if( mitk::BaseRenderer::GetInstance((*iter))->GetSliceNavigationController()
        ->GetViewDirection() == mitk::SliceNavigationController::ViewDirection::Axial )
      {
        m_AxialBR = mitk::BaseRenderer::GetInstance((*iter));
      }
    }
  }

  SlicedGeometry3D* slicedWorldGeometry
    = dynamic_cast<SlicedGeometry3D*>
    (m_AxialBR->GetSliceNavigationController()->GetCreatedWorldGeometry()->GetGeometry3D(m_TimeStep));

  int numberOfSlices = slicedWorldGeometry->GetSlices();

  const mitk::Geometry2D* currentGeo = m_AxialBR->GetCurrentWorldGeometry2D();
  mitk::Point3D origin = currentGeo->GetOrigin();

  int sliceValue = wiiMoteIREvent->GetSliceNavigationValue();

  if(sliceValue > 0)
  {
    if(m_InitialScrollValue == 0)
    {
      m_InitialScrollValue = sliceValue;
    }
    else if(std::abs(m_InitialScrollValue - sliceValue) > 10)
    {
      if(m_UpdateFrequency == UPDATEFREQUENCY /* 5 */)
      {
        int steppingValue;
        int currentPos = origin.GetElement(2);

        if(sliceValue < m_InitialScrollValue)
        {
          /* steppingValue = m_InitialScrollValue - sliceValue;     */
          steppingValue = currentGeo->GetSpacing()[2];
          origin.SetElement(2, currentPos-steppingValue);
        }
        else if(sliceValue > m_InitialScrollValue)
        {
          /* steppingValue = sliceValue - m_InitialScrollValue;*/
          steppingValue = currentGeo->GetSpacing()[2];
          origin.SetElement(2, currentPos+steppingValue);
        }

        m_UpdateFrequency = 0;
      }
      else
      {
        m_UpdateFrequency++;
      }
    }
  }
  else
  {
    m_InitialScrollValue = 0;
  }

  m_AxialBR->GetSliceNavigationController()->SelectSliceByPoint(origin);

  mitk::RenderingManager::GetInstance()
    ->RequestUpdateAll();

  return true;
}

bool mitk::WiiMoteVtkCameraController::ResetView(mitk::Action *a, const mitk::StateEvent *e)
{
  //reset the camera, so that the objects shown in the scene can be seen.
  const mitk::VtkPropRenderer* glRenderer = dynamic_cast<const mitk::VtkPropRenderer*>(m_Renderer);
  if (glRenderer)
  {
    vtkRenderer* vtkRenderer = glRenderer->GetVtkRenderer();
    mitk::DataStorage* ds = m_Renderer->GetDataStorage();
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

bool mitk::WiiMoteVtkCameraController::InitCalibration(mitk::Action *a, const mitk::StateEvent *e)
{
  // to initialize the values with its counterpart
  // is essential. The reason is that through calibration
  // the values will increase or decrease depending on
  // semantics:
  // the max will try to reach the XMAX (from XMIN)
  // the min will try to reach the XMIN (from XMAX)
  // i.e. in the calibration process they move
  // into their opposite direction to create an
  // intervall that defines the boundaries
  m_SensitivityX = 0;
  m_SensitivityXMAX = XMIN;
  m_SensitivityXMIN = XMAX;
  m_SensitivityY = 0;
  m_SensitivityYMAX = YMIN;
  m_SensitivityYMIN = YMAX;

  this->m_Calibrated = false;
  MITK_INFO << "Starting calibration - other wiimote functionality deactivated.";
  return true;
}

bool mitk::WiiMoteVtkCameraController::Calibration(mitk::Action *a, const mitk::StateEvent *e)
{
  const mitk::WiiMoteCalibrationEvent* WiiMoteCalibrationEvent;

  if(!(WiiMoteCalibrationEvent = dynamic_cast<const mitk::WiiMoteCalibrationEvent*>(e->GetEvent())))
  {
    MITK_ERROR << "Not a WiiMote Event!";
    return false;
  }

  double tempX(WiiMoteCalibrationEvent->GetXCoordinate());
  double tempY(WiiMoteCalibrationEvent->GetYCoordinate());

  MITK_INFO << "Raw X: " << tempX;
  MITK_INFO << "Raw Y: " << tempY;

  // checks first whether the incoming data is valid
  if(XMIN < tempX && tempX < XMAX)
  {
    if(tempX > m_SensitivityXMAX)
    {
      m_SensitivityXMAX = tempX;
    }
    else if(tempX < m_SensitivityXMIN)
    {
      m_SensitivityXMIN = tempX;
    }
  }

  if(YMIN < tempY && tempY < YMAX)
  {
    if(tempY > m_SensitivityYMAX)
    {
      m_SensitivityYMAX = tempY;
    }
    else if(tempY < m_SensitivityYMIN)
    {
      m_SensitivityYMIN = tempY;
    }
  }

  return true;
}

bool mitk::WiiMoteVtkCameraController::FinishCalibration(mitk::Action *a, const mitk::StateEvent *e)
{
  // checks if one of the properties was not set at all during the calibration
  // should that happen, the computation will not be executed
  if( m_SensitivityXMAX != XMIN
    && m_SensitivityXMIN != XMAX
    && m_SensitivityYMAX != YMIN
    && m_SensitivityYMIN != YMAX )
  {
    // computation of the sensitivity out of the calibration data
    m_SensitivityX = XMAX / (m_SensitivityXMAX - m_SensitivityXMIN);
    m_SensitivityX *= CALIBRATIONFACTORX;

    m_SensitivityY = YMAX / (m_SensitivityYMAX - m_SensitivityYMIN);
    m_SensitivityY *= CALIBRATIONFACTORY;

    this->m_Calibrated = true;
  }

  if(!m_Calibrated)
  {
    MITK_INFO << "Calibration was unsuccesful - "
      << "please repeat the process and move in all directions!";
  }
  else
  {
    MITK_INFO << "Ending calibration - other wiimote functionality reactivated.";
  }

  return m_Calibrated;
}

