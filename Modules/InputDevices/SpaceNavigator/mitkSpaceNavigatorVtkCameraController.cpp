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

#include "mitkSpaceNavigatorVtkCameraController.h"
#include "mitkVtkPropRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkTransform.h"
#include "mitkNumericTypes.h"
#include "mitkInteractionConst.h"
#include "mitkStateEvent.h"
#include "mitkGlobalInteraction.h"
#include "vtkMath.h"

#include <mitkSpaceNavigatorEvent.h>

const double TRANSLATIONSENSITIVITY = 2.0;
const double ROTATIONSENSIVITY = 3.5;
const double ANGLESENSITIVITY = 3.5;


mitk::SpaceNavigatorVtkCameraController::SpaceNavigatorVtkCameraController()
: CameraController("SpaceNavigatorInteraction")
{
  //connect method this->OnSpaceNavigatorEvent to StateMachineEventMechanism
  CONNECT_ACTION(AcONSPACENAVIGATORMOUSEINPUT, OnSpaceNavigatorEvent);
  CONNECT_ACTION(AcONPACENAVIGATORKEYDOWN, OnSpaceNavigatorKeyDown);
  m_ClippingRangeIsSet = false;
}

mitk::SpaceNavigatorVtkCameraController::~SpaceNavigatorVtkCameraController()
{
}

bool mitk::SpaceNavigatorVtkCameraController::OnSpaceNavigatorEvent(mitk::Action* a, const mitk::StateEvent* e)
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

  if(!m_ClippingRangeIsSet)
    vtkCam->SetClippingRange(0.1, 1000000);

  const mitk::SpaceNavigatorEvent* snevent = dynamic_cast<const mitk::SpaceNavigatorEvent*>(e->GetEvent());
  if (snevent == NULL)
  {
    MITK_ERROR <<"Wrong event for SpaceNavigatorVtkCameraController!";
    return false;
  }

  //get the information from the mouse
  mitk::Vector3D translation = snevent->GetTranslation();
  mitk::Vector3D rotation = snevent->GetRotation();
  mitk::ScalarType angle = snevent->GetAngle();

  //output for debug
  //std::cout<<"translation: "<<translation[0]<<" "<<translation[1]<<" "<<translation[2]<<" "<<"\n";
  //std::cout<<"rotation: "<<rotation[0]<<" "<<rotation[1]<<" "<<rotation[2]<<" "<<"\n";
  //std::cout<<"angle: "<<angle<<"\n \n";


  //empirically gained max and min valued from driver:
  // MAXIMA:
  // translation:  0.83    1.17    1.02
  // rotation:     1.00    1.00    1.00
  // angle:        0.83
  // MINIMA:
  // translation: -1.00   -1.03   -0.88
  // rotation:    -1.00   -1.00   -1.00
  // angle:        0.00

  //compute current sensitivity according to current BoundingBox of the whole scene!
  double sceneSensivity = 1.0;

  mitk::DataStorage* ds = m_Renderer->GetDataStorage();
  mitk::BoundingBox::Pointer bb = ds->ComputeBoundingBox();
  mitk::BoundingBox::AccumulateType length = bb->GetDiagonalLength2();
  if (length > 0.00001)//if length not zero
    sceneSensivity *= 100.0 / (sqrt(length)) ;

  //sensivity to adapt to mitk speed
  translation *= sceneSensivity * TRANSLATIONSENSITIVITY;
  rotation *= sceneSensivity * ROTATIONSENSIVITY;
  angle *= sceneSensivity * ANGLESENSITIVITY;

  //compute the global space coordinates from the relative mouse coordinate
  //first we need the position of the camera
  mitk::Vector3D camPosition;
  double camPositionTemp[3];
  vtkCam->GetPosition(camPositionTemp);
  camPosition[0] = camPositionTemp[0]; camPosition[1] = camPositionTemp[1]; camPosition[2] = camPositionTemp[2];

  //then the upvector of the camera
  mitk::Vector3D upCamVector;
  double upCamTemp[3];
  vtkCam->GetViewUp(upCamTemp);
  upCamVector[0] = upCamTemp[0]; upCamVector[1] = upCamTemp[1]; upCamVector[2] = upCamTemp[2];
  upCamVector.Normalize();

  //then the vector to which the camera is heading at (focalpoint)
  mitk::Vector3D focalPoint;
  double focalPointTemp[3];
  vtkCam->GetFocalPoint(focalPointTemp);
  focalPoint[0] = focalPointTemp[0]; focalPoint[1] = focalPointTemp[1]; focalPoint[2] = focalPointTemp[2];
  mitk::Vector3D focalVector;
  focalVector = focalPoint - camPosition;
  focalVector.Normalize();

  //orthogonal vector to focalVector and upCamVector
  mitk::Vector3D crossVector;
  crossVector = CrossProduct(upCamVector, focalVector);
  crossVector.Normalize();

  //now we have the current orientation so we can adapt it according to the current information, which we got from the TDMouse

  //new position of the camera:
  //left/right = camPosition + crossVector * translation[0];
  mitk::Vector3D vectorX = crossVector * -translation[0]; //changes the magnitude, not the direction
  double nextCamPosition[3];
  nextCamPosition[0] = camPosition[0] + vectorX[0];
  nextCamPosition[1] = camPosition[1] + vectorX[1];
  nextCamPosition[2] = camPosition[2] + vectorX[2];

  //now the up/down movement
  mitk::Vector3D vectorY = upCamVector * translation[1]; //changes the magnitude, not the direction
  nextCamPosition[0] += vectorY[0];
  nextCamPosition[1] += vectorY[1];
  nextCamPosition[2] += vectorY[2];

  //forward/backward movement
  mitk::Vector3D vectorZ = focalVector * -translation[2]; //changes the magnitude, not the direction
  nextCamPosition[0] += vectorZ[0];
  nextCamPosition[1] += vectorZ[1];
  nextCamPosition[2] += vectorZ[2];

  //set the next position
  double nextPosition[3];
  nextPosition[0] = nextCamPosition[0]; nextPosition[1] = nextCamPosition[1]; nextPosition[2] = nextCamPosition[2];
  vtkCam->SetPosition(nextPosition);

  //adapt the focal point the same way
  double currentFocalPoint[3], nextFocalPoint[3];
  vtkCam->GetFocalPoint(currentFocalPoint);
  nextFocalPoint[0] = currentFocalPoint[0] + vectorX[0] + vectorY[0] + vectorZ[0];
  nextFocalPoint[1] = currentFocalPoint[1] + vectorX[1] + vectorY[1] + vectorZ[1]; ;
  nextFocalPoint[2] = currentFocalPoint[2] + vectorX[2] + vectorY[2] + vectorZ[2];
  vtkCam->SetFocalPoint(nextFocalPoint);


  //now adapt the rotation of the mouse and adapt the camera according to it

  //Pitch:
  //Rotate the focal point about the cross product of the view up vector and the direction of
  //projection, centered at the camera's position.
  vtkCam->Pitch(rotation[0]*angle);

  //Yaw:
  //Rotate the focal point about the view up vector centered at the camera's position.
  //Note that the view up vector is not necessarily perpendicular to the direction of projection.
  vtkCam->Yaw(rotation[1]*angle);

  //Roll:
  //Rotate the camera about the direction of projection.
  vtkCam->Roll(-rotation[2]*angle * 1.5);//*1.5 to speed up the rotation[2] a little bit

  //Recompute the ViewUp vector to force it to be perpendicular to camera->focalpoint vector.
  //Unless you are going to use Yaw or Azimuth on the camera, there is no need to do this.
  vtkCam->OrthogonalizeViewUp();

  //no zooming, only translating to the front or back
  // dolly: Move the position of the camera along the direction
  // of projection. Moving towards the focal point (e.g., greater
  // than 1) is a dolly-in, moving away from the focal point
  // (e.g., less than 1) is a dolly-out.
  //double distance = ((tdevent->GetTranslation())[1] / 10.0);//make it less sensitive in comparison to translation and rotatipn
  //vtkCam->Dolly(1.0 + distance );


  //Reset the camera clipping range based on the bounds of the visible actors.
  //This ensures that no props are cut off
  vtkRenderer->ResetCameraClippingRange();

  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(mitk::GlobalInteraction::GetInstance()->GetFocus()->GetRenderWindow());

  return true;
}


bool mitk::SpaceNavigatorVtkCameraController::OnSpaceNavigatorKeyDown(mitk::Action* a, const mitk::StateEvent* e)
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

    mitk::Point3D middle =bb->GetCenter();
    vtkRenderer->GetActiveCamera()->SetFocalPoint(middle[0],middle[1],middle[2]);

    vtkRenderer->ResetCamera();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return true;
  }
  return false;
}
