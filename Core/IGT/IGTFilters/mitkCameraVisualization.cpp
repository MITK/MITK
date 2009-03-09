/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkCameraVisualization.h"

#include "vtkCamera.h"

#include "mitkPropertyList.h"
#include "mitkProperties.h"

namespace mitk{

  CameraVisualization::CameraVisualization()
  {
    // initialize members
    m_Renderer = NULL;
    m_DirectionOfProjectionInToolCoordinates[0] = 0;
    m_DirectionOfProjectionInToolCoordinates[1] = 0;
    m_DirectionOfProjectionInToolCoordinates[2] = -1;
    m_ViewUpInToolCoordinates[0] = 1;
    m_ViewUpInToolCoordinates[1] = 0;
    m_ViewUpInToolCoordinates[2] = 0;
    m_FocalLength = 10;
    m_ViewAngle = 30;
  }

  CameraVisualization::~CameraVisualization()
  {

  }

  void CameraVisualization::GenerateData()
  {
    // check if renderer was set
    if (m_Renderer)
    {
      const NavigationData* navigationData = this->GetInput();
      // get position from NavigationData
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
      mitk::TransferMatrix(quatTransform->GetMatrix(), m);

      Vector3D directionOfProjection = m*m_DirectionOfProjectionInToolCoordinates;
      directionOfProjection.Normalize();
      Point3D focalPoint = cameraPosition + m_FocalLength*directionOfProjection;
      // compute current view up vector
      Vector3D viewUp = m*m_ViewUpInToolCoordinates;

      m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetPosition(cameraPosition[0],cameraPosition[1],cameraPosition[2]);
      m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetFocalPoint(focalPoint[0],focalPoint[1],focalPoint[2]);
      m_Renderer->GetVtkRenderer()->GetActiveCamera()->ComputeViewPlaneNormal();
      m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetViewUp(viewUp[0],viewUp[1],viewUp[2]);
      m_Renderer->GetVtkRenderer()->GetActiveCamera()->OrthogonalizeViewUp();
      m_Renderer->GetVtkRenderer()->GetActiveCamera()->SetViewAngle(m_ViewAngle);
      //double* bounds = m_Renderer->GetVtkRenderer()->ComputeVisiblePropBounds();
      m_Renderer->GetVtkRenderer()->ResetCameraClippingRange();
      //m_Renderer->GetVtkRenderer()->ResetCamera();

      m_Renderer->RequestUpdate();

      std::cout<<"Camera Position: "<<cameraPosition<<std::endl;
      std::cout<<"Focal Point: "<<focalPoint<<std::endl;
      std::cout<<"View Up: "<<viewUp<<std::endl;
      std::cout<<"Direction of Projection: "<<directionOfProjection<<std::endl;
      std::cout<<"View Angle: "<<m_ViewAngle<<std::endl<<std::endl;
    }
    else
    {
      itkExceptionMacro(<< "Renderer was not properly set");
    }
  }

  void CameraVisualization::SetRenderer(VtkPropRenderer::Pointer renderer)
  {
    m_Renderer = renderer;
    //m_Renderer->GetVtkRenderer()->GetActiveCamera()->Zoom(0.1);
  }

  const BaseRenderer* CameraVisualization::GetRenderer()
  {
    return m_Renderer;
  }

  void CameraVisualization::SetParameters( const mitk::PropertyList* p )
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

  mitk::PropertyList::ConstPointer CameraVisualization::GetParameters() const
  {
    mitk::PropertyList::Pointer p = mitk::PropertyList::New();
    p->SetProperty("CameraVisualization_DirectionOfProjectionInToolCoordinates", mitk::Vector3DProperty::New(this->GetDirectionOfProjectionInToolCoordinates()));  // store DirectionOfProjectionInToolCoordinates parameter
    p->SetProperty("CameraVisualization_ViewUpInToolCoordinates", mitk::Vector3DProperty::New(this->GetViewUpInToolCoordinates()));  // store ViewUpInToolCoordinates parameter
    p->SetProperty("CameraVisualization_FocalLength", mitk::Vector3DProperty::New(this->GetFocalLength()));  // store FocalLength parameter
    p->SetProperty("CameraVisualization_ViewAngle", mitk::Vector3DProperty::New(this->GetViewAngle()));  // store ViewAngle parameter
    return mitk::PropertyList::ConstPointer(p);
  }
}
