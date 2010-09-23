#include "mitkWiiMoteVtkCameraController.h"

#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "mitkStateEvent.h"

#include "mitkBaseRenderer.h"
#include "mitkVtkPropRenderer.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"

const double TRANSLATIONSENSITIVITY = 2.0;

mitk::WiiMoteVtkCameraController::WiiMoteVtkCameraController() 
: CameraController("WiiMoteInteraction")
, m_ClippingRangeIsSet(false)
{
  CONNECT_ACTION(mitk::AcONWIIMOTEINPUT, OnWiiMoteInput);
  CONNECT_ACTION(mitk::AcONWIIMOTEHOMEBUTTON, OnWiiMoteHomeButton);
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

  const mitk::WiiMoteEvent* wiiMoteEvent;

  if(wiiMoteEvent = dynamic_cast<const mitk::WiiMoteEvent*>(e->GetEvent()))
  {
  //  // testing
  //  MITK_INFO << "Getting WiiMote input: ";
  //  MITK_INFO << "Vector: " << wiiMoteEvent->GetMovementVector();
  }
  else
  {
    MITK_ERROR << "Not a WiiMote Event!";
    return false;
  }

  // get data from the Wiimote
  mitk::Vector2D tempMovementVector(wiiMoteEvent->GetMovementVector());
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
  movementVector *= sceneSensivity * TRANSLATIONSENSITIVITY;

  // inverts y direction to simulate a 3D View
  // x direction is already inverted
  movementVector[1] *= -1;
  vtkCam->Elevation((double)movementVector[1]);
  vtkCam->Azimuth((double)movementVector[0]);

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

  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(mitk::GlobalInteraction::GetInstance()->GetFocus()->GetRenderWindow());

  return true;
}

bool mitk::WiiMoteVtkCameraController::OnWiiMoteHomeButton(mitk::Action *a, const mitk::StateEvent *e)
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

