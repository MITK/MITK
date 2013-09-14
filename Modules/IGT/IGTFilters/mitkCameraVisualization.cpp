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

#include "mitkCameraVisualization.h"

#include "vtkCamera.h"

#include "mitkPropertyList.h"
#include "mitkProperties.h"




mitk::CameraVisualization::CameraVisualization(): NavigationDataToNavigationDataFilter(),
m_Renderer(NULL), m_FocalLength(10.0)
{
  // initialize members
  m_DirectionOfProjectionInToolCoordinates[0] = 0;
  m_DirectionOfProjectionInToolCoordinates[1] = 0;
  m_DirectionOfProjectionInToolCoordinates[2] = -1;
  m_ViewUpInToolCoordinates[0] = 1;
  m_ViewUpInToolCoordinates[1] = 0;
  m_ViewUpInToolCoordinates[2] = 0;
}


mitk::CameraVisualization::~CameraVisualization()
{

}


void mitk::CameraVisualization::GenerateData()
{
  // check if renderer was set
  if (m_Renderer.IsNull())
    itkExceptionMacro(<< "Renderer was not properly set");

  /* update outputs with tracking data from tools */
  unsigned int numberOfInputs = this->GetNumberOfInputs();
  for (unsigned int i = 0; i < numberOfInputs ; ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);

    if (input->IsDataValid() == false)
    {
      continue;
    }
    output->Graft(input); // First, copy all information from input to output
  }

  const NavigationData* navigationData = this->GetInput();
  // get position from NavigationData to move the camera to this position
  Point3D cameraPosition = navigationData->GetPosition();

  //calculate the transform from the quaternions
  static itk::QuaternionRigidTransform<double>::Pointer quatTransform = itk::QuaternionRigidTransform<double>::New();

  mitk::NavigationData::OrientationType orientation = navigationData->GetOrientation();
  // convert mitk::Scalartype quaternion to double quaternion because of itk bug
  vnl_quaternion<double> doubleQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.r());
  quatTransform->SetIdentity();
  quatTransform->SetRotation(doubleQuaternion);
  quatTransform->Modified();

  /* because of an itk bug, the transform can not be calculated with float datatype.
  To use it in the mitk geometry classes, it has to be transfered to mitk::ScalarType which is float */
  static AffineTransform3D::MatrixType m;
  //mitk::TransferMatrix(quatTransform->GetMatrix(), m);
  m = navigationData->GetOrientation().rotation_matrix_transpose();


  Vector3D directionOfProjection = m*m_DirectionOfProjectionInToolCoordinates;
  directionOfProjection.Normalize();
  Point3D focalPoint = cameraPosition + m_FocalLength*directionOfProjection;
  // compute current view up vector
  Vector3D viewUp = m*m_ViewUpInToolCoordinates;

  m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetPosition(cameraPosition[0],cameraPosition[1],cameraPosition[2]);
  m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetFocalPoint(focalPoint[0],focalPoint[1],focalPoint[2]);
  m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetViewUp(viewUp[0],viewUp[1],viewUp[2]);
  m_Renderer->GetVtkRenderer()->ResetCameraClippingRange();

  m_Renderer->RequestUpdate();
}


void mitk::CameraVisualization::SetRenderer(mitk::BaseRenderer*  renderer)
{
  m_Renderer = renderer;
}


const mitk::BaseRenderer* mitk::CameraVisualization::GetRenderer()
{
  return m_Renderer;
}


void mitk::CameraVisualization::SetParameters( const mitk::PropertyList* p )
{
  if (p == NULL)
    return;
  mitk::Vector3D doP;
  if (p->GetPropertyValue<mitk::Vector3D>("CameraVisualization_DirectionOfProjectionInToolCoordinates", doP) == true)  // search for DirectionOfProjectionInToolCoordinates parameter
    this->SetDirectionOfProjectionInToolCoordinates(doP);   // apply if found;
  mitk::Vector3D vUp;
  if (p->GetPropertyValue<mitk::Vector3D>("CameraVisualization_ViewUpInToolCoordinates", vUp) == true)  // search for ViewUpInToolCoordinates parameter
    this->SetViewUpInToolCoordinates(vUp);   // apply if found;
  float fL;
  if (p->GetPropertyValue<float>("CameraVisualization_FocalLength", fL) == true)  // search for FocalLength parameter
    this->SetFocalLength(fL);   // apply if found;
  float vA;
  if (p->GetPropertyValue<float>("CameraVisualization_ViewAngle", vA) == true)  // search for ViewAngle parameter
    this->SetFocalLength(vA);   // apply if found;
}


mitk::PropertyList::ConstPointer mitk::CameraVisualization::GetParameters() const
{
  mitk::PropertyList::Pointer p = mitk::PropertyList::New();
  p->SetProperty("CameraVisualization_DirectionOfProjectionInToolCoordinates", mitk::Vector3DProperty::New(this->GetDirectionOfProjectionInToolCoordinates()));  // store DirectionOfProjectionInToolCoordinates parameter
  p->SetProperty("CameraVisualization_ViewUpInToolCoordinates", mitk::Vector3DProperty::New(this->GetViewUpInToolCoordinates()));  // store ViewUpInToolCoordinates parameter
  p->SetProperty("CameraVisualization_FocalLength", mitk::Vector3DProperty::New(this->GetFocalLength()));  // store FocalLength parameter
  return mitk::PropertyList::ConstPointer(p);
}
