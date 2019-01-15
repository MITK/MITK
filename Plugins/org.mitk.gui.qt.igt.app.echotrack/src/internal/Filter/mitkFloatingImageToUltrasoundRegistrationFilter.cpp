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

#include "mitkFloatingImageToUltrasoundRegistrationFilter.h"
#include <itkScalableAffineTransform.h>
#include <itkScalableAffineTransform.hxx>


mitk::FloatingImageToUltrasoundRegistrationFilter::FloatingImageToUltrasoundRegistrationFilter()
: mitk::NavigationDataPassThroughFilter(),
  m_Segmentation(nullptr),
  m_TransformMarkerCSToSensorCS(mitk::AffineTransform3D::New()),
  m_TransformMarkerCSToFloatingImageCS(mitk::AffineTransform3D::New()),
  m_TransformUSimageCSToTrackingCS(mitk::AffineTransform3D::New()),
  m_TransformCTimageIndexToWorld(mitk::AffineTransform3D::New()),
  m_TrackedUltrasoundActive(false)
{
  m_SurfaceGeometry = mitk::Geometry3D::New();
  m_PointSet = nullptr;
  m_CTimage = mitk::Image::New();
}

mitk::FloatingImageToUltrasoundRegistrationFilter::~FloatingImageToUltrasoundRegistrationFilter()
{
}

void mitk::FloatingImageToUltrasoundRegistrationFilter::InitializeTransformationMarkerCSToSensorCS( bool useNdiTracker)
{
  //The following calculations are related to the 3mm | 15mm fiducial configuration

  m_TransformMarkerCSToSensorCS = mitk::AffineTransform3D::New();

  if (useNdiTracker) // Use the NDI disc tracker for performing the CT-to-US registration:
  {
    MITK_INFO << "Use NDI disc tracker for performing the CT-to-US-registration";
    mitk::Vector3D translationNDI;
    translationNDI[0] = 15.000;
    translationNDI[1] = 8.000;
    translationNDI[2] = -2.500; // use -2.500 instead of 0.000 for the z translation if working with v2 of the
                                // sensor adapter.

    m_TransformMarkerCSToSensorCS->SetOffset(translationNDI);

    // Quaternion (x, y, z, r) --> n = (0,0,1) --> q(0,0,sin(90°),cos(90°))
    mitk::Quaternion qNDI(0, 0, 1, 0); // corresponding to a rotation of 180° around the normal z-axis.
                                     // .transpose() is needed for changing the rows and the columns of the returned rotation_matrix_transpose
    vnl_matrix_fixed<double, 3, 3> vnl_rotation = qNDI.rotation_matrix_transpose().transpose(); // :-)
    mitk::Matrix3D rotationMatrix;

    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        rotationMatrix[i][j] = vnl_rotation[i][j];
      }
    }

    m_TransformMarkerCSToSensorCS->SetMatrix(rotationMatrix);
  }
  else // Use the polhemus RX2 tracker for performing the CT-to-US registration:
  {
    MITK_INFO << "Use Polhemus RX2 tracker for performing the CT-to-US-registration";
    mitk::Vector3D translationPolhemus;
    translationPolhemus[0] = -18.175;
    translationPolhemus[1] = 15.000;
    translationPolhemus[2] = 10.501; // considering a distance from the base plate of 0.315 inch and not 0.313 inch
                             // use 10.501 instead of 8.001 for the z translation if working with v2 of the
                             // sensor adapter.

    m_TransformMarkerCSToSensorCS->SetOffset(translationPolhemus);

    // Quaternion (x, y, z, r) --> n = (1,0,0) --> q(sin(90°),0,0,cos(90°))
    mitk::Quaternion q1(1, 0, 0, 0); // corresponding to a rotation of 180° around the normal x-axis.
    // .transpose() is needed for changing the rows and the columns of the returned rotation_matrix_transpose
    vnl_matrix_fixed<double, 3, 3> vnl_rotation = q1.rotation_matrix_transpose().transpose(); // :-)
    mitk::Matrix3D rotationMatrix;

    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        rotationMatrix[i][j] = vnl_rotation[i][j];
      }
    }

    m_TransformMarkerCSToSensorCS->SetMatrix(rotationMatrix);
  }
  //The transformation from the sensor-CS to the marker-CS is calculated now.
}

void mitk::FloatingImageToUltrasoundRegistrationFilter::SetSegmentation(mitk::DataNode::Pointer segmentationNode, mitk::Image::Pointer ctimage)
{
  m_Segmentation = segmentationNode;
  m_TransformCTimageIndexToWorld = m_Segmentation->GetData()->GetGeometry()->GetIndexToWorldTransform();
  m_CTimage = ctimage;
}

void mitk::FloatingImageToUltrasoundRegistrationFilter::SetSurface(mitk::DataNode::Pointer surfaceNode)
{
  m_Surface = surfaceNode;
  m_SurfaceGeometry = m_Surface->GetData()->GetGeometry();
}

void mitk::FloatingImageToUltrasoundRegistrationFilter::SetPointSet(mitk::DataNode::Pointer pointSetNode)
{
  m_PointSet = pointSetNode;
}

void mitk::FloatingImageToUltrasoundRegistrationFilter::SetTransformMarkerCSToFloatingImageCS(mitk::AffineTransform3D::Pointer transform)
{
  m_TransformMarkerCSToFloatingImageCS = transform;
}

void mitk::FloatingImageToUltrasoundRegistrationFilter::SetTransformUSimageCSToTrackingCS(mitk::AffineTransform3D::Pointer transform)
{
  m_TransformUSimageCSToTrackingCS = transform;
}

void mitk::FloatingImageToUltrasoundRegistrationFilter::GenerateData()
{
  Superclass::GenerateData();

  if (m_TrackedUltrasoundActive)
  {
    MITK_WARN << "The CT-to-US-registration is not supported by tracked ultrasound, yet.";
    return;
  }

  //IMPORTANT --- Hard coded --- First device = EM-Sensor | eventually second device = needle
  mitk::NavigationData::Pointer transformSensorCSToTracking = this->GetOutput(0);

  // cancel, if the EM-sensor is currently not being tracked
  if (!transformSensorCSToTracking->IsDataValid())
  {
    return;
  }

  //We start the transformation with a new transform:
  mitk::AffineTransform3D::Pointer totalTransformation = mitk::AffineTransform3D::New();
  totalTransformation->SetIdentity();
  //Compose it with the inverse transform of marker-CS to floating image-CS:
  totalTransformation->Compose(this->GetInverseTransform(m_TransformMarkerCSToFloatingImageCS));
  //Compose this with the inverse transform of EM-sensor-CS to marker-CS:
  totalTransformation->Compose(m_TransformMarkerCSToSensorCS);
  //Compose this with the transform of the sensor-CS to Tracking-CS:
  totalTransformation->Compose(transformSensorCSToTracking->GetAffineTransform3D());
  //Compose this with the inverse transform  of USimage-CS to tracking-CS:
  totalTransformation->Modified();

  //Finally, set the total transformation (from floatingImage-CS to US-image-CS
  // to the selected floatingImageSurface:

  //m_Segmentation->GetData()->GetGeometry()->SetIndexToWorldTransform(totalTransformation);
  //m_Segmentation->Modified();

  //m_CTimage->GetGeometry()->SetIndexToWorldTransform(totalTransformation);
  //m_CTimage->Modified();

  m_Surface->GetData()->GetGeometry()->SetIndexToWorldTransform(totalTransformation);
  m_Surface->Modified();

  m_PointSet->GetData()->GetGeometry()->SetIndexToWorldTransform(totalTransformation);
  m_PointSet->Modified();
}

mitk::AffineTransform3D::Pointer mitk::FloatingImageToUltrasoundRegistrationFilter::GetInverseTransform(mitk::AffineTransform3D::Pointer transform)
{
  mitk::AffineTransform3D::Pointer inverseTransform = mitk::AffineTransform3D::New();
  mitk::AffineTransform3D::Pointer inverse = dynamic_cast<mitk::AffineTransform3D*>(transform->GetInverseTransform().GetPointer());

  //itk::SmartPointer<itk::ScalableAffineTransform<mitk::ScalarType, 3U>> inverse = dynamic_cast<itk::ScalableAffineTransform<mitk::ScalarType, 3U>*> (transform->GetInverseTransform().GetPointer()); //itkScalableAffineTransform_hxx

  if (inverse.IsNull())
  {
    MITK_WARN << "Could not get inverse transform of mitk::AffineTransform3D. Returning nullptr";
    return nullptr;
  }

  inverseTransform->SetOffset(inverse->GetOffset());
  inverseTransform->SetMatrix(inverse->GetMatrix());

  return inverseTransform;
}
