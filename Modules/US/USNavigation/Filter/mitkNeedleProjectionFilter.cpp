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

// MITK
#include "mitkNeedleProjectionFilter.h"
#include <mitkPlaneGeometry.h>

// VTK
#include <vtkPlane.h>


mitk::NeedleProjectionFilter::NeedleProjectionFilter()
  : m_Projection(mitk::PointSet::New()),
    m_OriginalPoints(mitk::PointSet::New()),
    m_SelectedInput(-1)
{
  // Tool Coordinates: First point - Tip of Needle, Second Point - 40 cm distance from needle
  for (int i = 0; i < 2; i++)
  {
    mitk::Point3D point;
    point.SetElement(0,0);
    point.SetElement(1,0);
    point.SetElement(2, i * 400);
    m_OriginalPoints->InsertPoint(i, point);
  }
}

mitk::NeedleProjectionFilter::~NeedleProjectionFilter()
{
}


void mitk::NeedleProjectionFilter::SelectInput(int i)
{
  if (i < 0) mitkThrow() << "Negative Input selected in NeedleProjectionFilter";
  if (! (static_cast<unsigned int>(i) < this->GetInputs().size())) mitkThrow() << "Selected input index is larger than actual number of inputs in NeedleProjectionFilter";
  m_SelectedInput = i;
}

void mitk::NeedleProjectionFilter::GenerateData()
{
  // copy the navigation data from the inputs to the outputs
  mitk::NavigationDataPassThroughFilter::GenerateData();

  // If no reference has been set yet, warn and abort
  if (m_SelectedInput == -1)
  {
    MITK_INFO << "No input has been selected in NeedleProjection Filter. Only forwarding NavigationData...";
    return;
  }

  // Cancel, if selected tool is currently not being tracked
  if (! GetInput(m_SelectedInput)->IsDataValid()) return;

  // Outputs have been updated, now to calculate the Projection
  // 1) Generate Pseudo-Geometry for Input
  mitk::AffineTransform3D::Pointer refTrans = this->NavigationDataToTransform(this->GetInput(m_SelectedInput));
  mitk::Geometry3D::Pointer refGeom = this->TransformToGeometry(refTrans);
  // 2) Transform Original Pointset
  m_OriginalPoints->SetGeometry(refGeom);
  // Update Projection (We do not clone, since we want to keep properties alive)
  m_Projection->SetPoint(0, m_OriginalPoints->GetPoint(0));
  m_Projection->SetPoint(1, m_OriginalPoints->GetPoint(1));
  // 3a) If no target Plane has been set, then leave it at that
  if (this->m_TargetPlane.IsNull())
    return;

  // 3b) else, calculate intersection with plane
  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
  plane->SetIndexToWorldTransform(m_TargetPlane);
  //plane->TransferItkToVtkTransform(); //included in SetIndexToWorldTransform

  double t;
  double x[3];
  // Points that define the needle vector
  double p1[3] = {m_OriginalPoints->GetPoint(0)[0], m_OriginalPoints->GetPoint(0)[1], m_OriginalPoints->GetPoint(0)[2]};
  double p2[3] = {m_OriginalPoints->GetPoint(1)[0], m_OriginalPoints->GetPoint(1)[1], m_OriginalPoints->GetPoint(1)[2]};
  // Center of image plane and it's normal
  double center[3] = {plane->GetCenter()[0], plane->GetCenter()[1], plane->GetCenter()[2]};
  double normal[3] = {plane->GetNormal()[0], plane->GetNormal()[1], plane->GetNormal()[2]};

  vtkPlane::IntersectWithLine(p1, p2, normal, center, t, x);

  // change (cut) needle path only if the needle points to the image plane;
  // otherwise the needle path direction would be changed pointing to the image plane
  if ( t >= 0 )
  {
    // Convert vtk to itk
    mitk::Point3D intersection;
    intersection[0] = x[0];
    intersection[1] = x[1];
    intersection[2] = x[2];

    // Replace distant point with image intersection
    m_Projection->SetPoint(1, intersection);
  }
}

mitk::AffineTransform3D::Pointer mitk::NeedleProjectionFilter::NavigationDataToTransform(const mitk::NavigationData * nd)
{
  mitk::AffineTransform3D::Pointer affineTransform = mitk::AffineTransform3D::New();
  affineTransform->SetIdentity();

  //calculate the transform from the quaternions
  static itk::QuaternionRigidTransform<double>::Pointer quatTransform = itk::QuaternionRigidTransform<double>::New();

  mitk::NavigationData::OrientationType orientation = nd->GetOrientation();
  // convert mitk::ScalarType quaternion to double quaternion because of itk bug
  vnl_quaternion<double> doubleQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.r());
  quatTransform->SetIdentity();
  quatTransform->SetRotation(doubleQuaternion);
  quatTransform->Modified();

  /* because of an itk bug, the transform can not be calculated with float data type.
  To use it in the mitk geometry classes, it has to be transfered to mitk::ScalarType which is float */
  static AffineTransform3D::MatrixType m;
  mitk::TransferMatrix(quatTransform->GetMatrix(), m);
  affineTransform->SetMatrix(m);

  /*set the offset by convert from itkPoint to itkVector and setting offset of transform*/
  mitk::Vector3D pos;
  pos.SetVnlVector(nd->GetPosition().GetVnlVector());
  affineTransform->SetOffset(pos);

  affineTransform->Modified();
  return affineTransform;
}

mitk::Geometry3D::Pointer mitk::NeedleProjectionFilter::TransformToGeometry(mitk::AffineTransform3D::Pointer transform){
  mitk::Geometry3D::Pointer g3d = mitk::Geometry3D::New();
  mitk::ScalarType scale[] = {1.0, 1.0, 1.0};
  g3d->SetSpacing(scale);
  g3d->SetIndexToWorldTransform(transform);
  //g3d->TransferItkToVtkTransform(); // update VTK Transform for rendering too //included in SetIndexToWorldTransform
  g3d->Modified();
  return g3d;
}
