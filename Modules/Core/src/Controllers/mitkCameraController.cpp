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

#include "mitkCameraController.h"
#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"
#include <vtkRenderWindowInteractor.h>
#include "vtkCommand.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"
#include <vtkTransform.h>
#include <vtkSmartPointer.h>

mitk::CameraController::CameraController() :
  BaseController(),
  m_Renderer(NULL)
{
}

mitk::CameraController::~CameraController()
{}

mitk::ScalarType mitk::CameraController::ComputeMaxParallelScale()
{
  double widthInMM = this->GetRenderer()->GetCurrentWorldPlaneGeometry()->GetExtentInMM(0);
  double heightInMM = this->GetRenderer()->GetCurrentWorldPlaneGeometry()->GetExtentInMM(1);

  double dispHeight = this->GetRenderer()->GetViewportSize()[1]; //in pixel!
  double dispWidth = this->GetRenderer()->GetViewportSize()[0];

  //To get the right zooming factor, we need to set the (half) height to the vtk camera using SetParallelScale.
  //However, it could be, that our picture is so wide or the display so small, that we cannot take the height of the picture.
  //For a wide picture, we have to take the width and adapt the width so that our image fits to the screen.
  //But we can only set the height. Therefore, if the width is the limiting factor, we need to get the ratio of scaling
  //for the width and multiply it with the height, so that we have a modified height and set this one. Believe us, we figured it out...
  if ((dispWidth / widthInMM) < (dispHeight / heightInMM))
  {
    heightInMM = widthInMM / dispWidth * dispHeight;
  }

  return heightInMM *0.5;
}

mitk::ScalarType mitk::CameraController::ComputeMaxAllowedParallelScale()
{
  return ComputeMaxParallelScale() * ALLOWED_TO_MAX_ZOOM_COF;
}

void mitk::CameraController::AdjustConstrainedCameraPosition(mitk::Point2D &planePoint)
{
  //TODO: GetExtentInMM is calculated wrong for rotated planes, e.g. crosshair rotation (bug 19105)
  double widthInMM = this->GetRenderer()->GetCurrentWorldPlaneGeometry()->GetExtentInMM(0);
  double heightInMM = this->GetRenderer()->GetCurrentWorldPlaneGeometry()->GetExtentInMM(1);

  mitk::Point2D dispSizeInMM = this->GetRenderer()->GetViewportSizeInMM();

  double xMin, xMax, yMin, yMax;

  //different calculation of min/max if display is lager/smaller than image.
  //note, that the plane Position defines the middle of the display but is in image coordinates
  //([0,0] is defined by the image, so planePosition can sometimes be negative).
  if (dispSizeInMM[0] > widthInMM)
  {
    xMin = widthInMM - 0.5 * dispSizeInMM[0];
    xMax = 0.5 * dispSizeInMM[0] ;
  }
  else
  {
    xMin = 0.5 * dispSizeInMM[0];
    xMax = widthInMM - 0.5 * dispSizeInMM[0];
  }

  if (dispSizeInMM[1] > heightInMM)
  {
    yMin = heightInMM - 0.5 * dispSizeInMM[1];
    yMax = 0.5 * dispSizeInMM[1] ;
  }
  else
  {
    yMin = 0.5 * dispSizeInMM[1];
    yMax = heightInMM - 0.5 * dispSizeInMM[1];
  }

  if (planePoint[0] < xMin) { planePoint[0] = xMin; }
  if (planePoint[1] < yMin) { planePoint[1] = yMin; }
  if (planePoint[0] > xMax) { planePoint[0] = xMax; }
  if (planePoint[1] > yMax) { planePoint[1] = yMax; }
}

void mitk::CameraController::SetViewToAnterior()
{
  this->SetStandardView(ANTERIOR);
}

void mitk::CameraController::SetViewToPosterior()
{
  this->SetStandardView(POSTERIOR);
}

void mitk::CameraController::SetViewToSinister()
{
  this->SetStandardView(SINISTER);
}

void mitk::CameraController::SetViewToDexter()
{
  this->SetStandardView(DEXTER);
}

void mitk::CameraController::SetViewToCranial()
{
  this->SetStandardView(CRANIAL);
}

void mitk::CameraController::SetViewToCaudal()
{
  this->SetStandardView(CAUDAL);
}

void mitk::CameraController::SetStandardView(mitk::CameraController::StandardView view)
{
  const mitk::VtkPropRenderer* glRenderer = dynamic_cast<const mitk::VtkPropRenderer*>(m_Renderer);
  if (glRenderer == NULL)
    return;
  vtkRenderer* vtkRenderer = glRenderer->GetVtkRenderer();
  assert(vtkRenderer);

  mitk::BoundingBox::Pointer bb;
  mitk::DataStorage* ds = m_Renderer->GetDataStorage();
  if (ds != NULL)
    bb = ds->ComputeBoundingBox();
  else
    return;

  if (m_Renderer->GetMapperID() == mitk::BaseRenderer::Standard3D)
  {
    //set up the view for the 3D render window. The views for 2D are set up in the mitkVtkPropRenderer
    mitk::Point3D middle = bb->GetCenter();
    vtkRenderer->GetActiveCamera()->SetFocalPoint(middle[0], middle[1], middle[2]);
    switch (view)
    {
    case ANTERIOR:
    case POSTERIOR:
    case SINISTER:
    case DEXTER:
      vtkRenderer->GetActiveCamera()->SetViewUp(0, 0, 1);
      break;
    case CRANIAL:
    case CAUDAL:
      vtkRenderer->GetActiveCamera()->SetViewUp(0, -1, 0);
      break;
    }
    switch (view)
    {
    case ANTERIOR:
      vtkRenderer->GetActiveCamera()->SetPosition(middle[0], -100000, middle[2]);
      break;
    case POSTERIOR:
      vtkRenderer->GetActiveCamera()->SetPosition(middle[0], +100000, middle[2]);
      break;
    case SINISTER:
      vtkRenderer->GetActiveCamera()->SetPosition(+100000, middle[1], middle[2]);
      break;
    case DEXTER:
      vtkRenderer->GetActiveCamera()->SetPosition(-100000, middle[1], middle[2]);
      break;
    case CRANIAL:
      vtkRenderer->GetActiveCamera()->SetPosition(middle[0], middle[1], 100000);
      break;
    case CAUDAL:
      vtkRenderer->GetActiveCamera()->SetPosition(middle[0], middle[1], -100000);
      break;
    }
    vtkRenderer->ResetCamera();

    vtkRenderer->ResetCameraClippingRange();
  }

  mitk::RenderingManager* rm = m_Renderer->GetRenderingManager();
  rm->RequestUpdateAll();
}

void mitk::CameraController::MoveCameraToPoint(const mitk::Point2D& planePoint, bool useConstraints)
{
  Point2D moveToPoint = planePoint;
  AdjustCameraToPlane(moveToPoint, useConstraints);

  this->Modified();
}

void mitk::CameraController::MoveBy(const mitk::Vector2D &moveVector)
{
  vtkCamera* camera = this->GetRenderer()->GetVtkRenderer()->GetActiveCamera();

  Point2D camPointDisplay;
  camPointDisplay[0] = this->GetRenderer()->GetVtkRenderer()->GetCenter()[0];
  camPointDisplay[1] = this->GetRenderer()->GetVtkRenderer()->GetCenter()[1];

  camPointDisplay += moveVector;

  Point2D camPointPlane;
  this->GetRenderer()->DisplayToPlane(camPointDisplay, camPointPlane);
  AdjustConstrainedCameraPosition(camPointPlane);

  Point3D worldPoint;
  this->GetRenderer()->GetCurrentWorldPlaneGeometry()->Map(camPointPlane, worldPoint);

  Vector3D cameraDir = camera->GetDirectionOfProjection();
  camera->SetFocalPoint(worldPoint.GetDataPointer());
  worldPoint -= cameraDir * camera->GetDistance();
  camera->SetPosition(worldPoint.GetDataPointer());

  this->Modified();
}

void mitk::CameraController::Zoom(ScalarType factor, const Point2D& zoomPointInMM)
{
  if (factor <= 0.0) return;
  if (this->GetRenderer()->GetMapperID() == BaseRenderer::Standard2D)
  {
    double parallelScale = this->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->GetParallelScale()/factor;
    if (this->GetRenderer()->GetConstrainZoomingAndPanning() && factor < 1.0)
    {
      double maxParallelScale = ComputeMaxAllowedParallelScale();
      if (maxParallelScale - parallelScale * factor  < mitk::eps)//this is not the famous 05-bug... Return if already near max zooming
        return;

      if (parallelScale > maxParallelScale)
      {
        parallelScale = maxParallelScale;
      }
    }
    this->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetParallelScale(parallelScale);
    //Move camera in a way that the clicked point stays visible on the display where it was.
    Point2D planePoint = GetCameraPositionOnPlane();

    // Check if zoom point inside image
    // If this is the case, adjust plane point accordingly
    double widthInMM = this->GetRenderer()->GetCurrentWorldPlaneGeometry()->GetExtentInMM(0);
    double heightInMM = this->GetRenderer()->GetCurrentWorldPlaneGeometry()->GetExtentInMM(1);
    if (zoomPointInMM[0] >= 0. && zoomPointInMM[0] < widthInMM && zoomPointInMM[1] >= 0. && zoomPointInMM[1] < heightInMM) {
      planePoint = planePoint + ((zoomPointInMM - planePoint) * (factor - 1));
    }

    AdjustConstrainedCameraPosition(planePoint);

    Point3D worldPoint;
    this->GetRenderer()->GetCurrentWorldPlaneGeometry()->Map(planePoint, worldPoint);
   
    vtkCamera* camera = this->GetRenderer()->GetVtkRenderer()->GetActiveCamera();

    Vector3D cameraDir = Vector3D(camera->GetDirectionOfProjection());
    camera->SetFocalPoint(worldPoint.GetDataPointer());
    worldPoint -= cameraDir * camera->GetDistance();
    camera->SetPosition(worldPoint.GetDataPointer());
  }
}

mitk::Point2D mitk::CameraController::GetCameraPositionOnPlane()
{
  Point2D CamPosOnPlane;
  CamPosOnPlane[0] = this->GetRenderer()->GetVtkRenderer()->GetCenter()[0];
  CamPosOnPlane[1] = this->GetRenderer()->GetVtkRenderer()->GetCenter()[1];
  this->GetRenderer()->DisplayToPlane(CamPosOnPlane, CamPosOnPlane);
  return CamPosOnPlane;
}

void mitk::CameraController::AdjustCameraToPlane()
{
  if (this->GetRenderer()->GetMapperID() == BaseRenderer::Standard2D)
  {
    AdjustCameraToPlane(GetCameraPositionOnPlane());
  }
}

void mitk::CameraController::AdjustCameraToPlane(const Point2D& PlanePoint, bool useConstraints)
{
  if (this->GetRenderer()->GetMapperID() == BaseRenderer::Standard2D)
  {
    Point2D _planePoint = PlanePoint;//PlanePoint is const...
    if (this->GetRenderer()->GetConstrainZoomingAndPanning())
    {
      double parallelScale = this->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->GetParallelScale();
      double maxParallelScale = ComputeMaxAllowedParallelScale();
      if (parallelScale > maxParallelScale)
      {
        this->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetParallelScale(maxParallelScale);
      }
      if (useConstraints) {
        AdjustConstrainedCameraPosition(_planePoint);
      }
    }
    const PlaneGeometry *planeGeometry = this->GetRenderer()->GetCurrentWorldPlaneGeometry();
    if (planeGeometry != NULL)
    {
      vtkCamera* camera = this->GetRenderer()->GetVtkRenderer()->GetActiveCamera();

      camera->SetViewUp(0, 1, 0); //set the view-up for the camera
      camera->SetPosition(_planePoint[0], _planePoint[1], 900000);
      camera->SetFocalPoint(_planePoint[0], _planePoint[1], 0);

      //Transform the camera to the current position (transversal, coronal and sagittal plane).
      //This is necessary, because the SetUserTransform() method does not manipulate the vtkCamera.
      //(Without not all three planes would be visible).
      vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
      vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
      Point3D origin;
      Vector3D right, bottom, normal;

      origin = planeGeometry->GetOrigin();
      right = planeGeometry->GetAxisVector(0); // right = Extent of Image in mm (worldspace)
      bottom = planeGeometry->GetAxisVector(1);
      normal = planeGeometry->GetNormal();

      right.Normalize();
      bottom.Normalize();
      normal.Normalize();

      matrix->SetElement(0, 0, right[0]);
      matrix->SetElement(1, 0, right[1]);
      matrix->SetElement(2, 0, right[2]);
      matrix->SetElement(0, 1, bottom[0]);
      matrix->SetElement(1, 1, bottom[1]);
      matrix->SetElement(2, 1, bottom[2]);
      matrix->SetElement(0, 2, normal[0]);
      matrix->SetElement(1, 2, normal[1]);
      matrix->SetElement(2, 2, normal[2]);
      matrix->SetElement(0, 3, origin[0]);
      matrix->SetElement(1, 3, origin[1]);
      matrix->SetElement(2, 3, origin[2]);
      matrix->SetElement(3, 0, 0.0);
      matrix->SetElement(3, 1, 0.0);
      matrix->SetElement(3, 2, 0.0);
      matrix->SetElement(3, 3, 1.0);

      trans->SetMatrix(matrix);
      //Transform the camera to the current position (transversal, coronal and sagittal plane).
      camera->ApplyTransform(trans);
    }
  }
}

void mitk::CameraController::Fit()
{
  if (this->GetRenderer()->GetMapperID() == BaseRenderer::Standard2D)
  {
    this->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetParallelScale(ComputeMaxParallelScale());

    this->GetRenderer()->GetVtkRenderer()->GetActiveCamera()
      ->SetClippingRange(0.1, 1000000);
    //Reason for huge range: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.

    Point2D planePoint;
    planePoint[0] = this->GetRenderer()->GetCurrentWorldPlaneGeometry()->GetExtentInMM(0)*0.5;
    planePoint[1] = this->GetRenderer()->GetCurrentWorldPlaneGeometry()->GetExtentInMM(1)*0.5;
    MoveCameraToPoint(planePoint);
  }
}

void mitk::CameraController::SetScaleFactorInMMPerDisplayUnit( ScalarType scale )
{
  if ( this->GetRenderer()->GetMapperID() != BaseRenderer::Standard2D )
  {
    return;
  }

  this->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetParallelScale(
    this->GetRenderer()->GetViewportSize()[1] * scale * 0.5 );

  this->Modified();

}
