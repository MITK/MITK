/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk headers
#include "mitkSurfaceModifier.h"
#include "mitkSurfaceToPointSetFilter.h"

// vtk headers
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

mitk::SurfaceModifier::SurfaceModifier()
{
  m_myRandomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
}

mitk::SurfaceModifier::~SurfaceModifier()
{
}

mitk::Point3D mitk::SurfaceModifier::PerturbePointAlongAxis(mitk::Point3D point, mitk::Vector3D axis, double variance)
{
  if (m_myRandomGenerator.IsNull())
    m_myRandomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
  mitk::Point3D returnValue;

  // normalize axis
  mitk::Vector3D normalizedAxis = axis;
  normalizedAxis.Normalize();

  // create noise
  double noise = m_myRandomGenerator->GetNormalVariate(0.0, variance);

  // std::cout <<point<<std::endl;
  // add noise to point along axis
  returnValue[0] = point[0] + axis[0] * noise;
  returnValue[1] = point[1] + axis[1] * noise;
  returnValue[2] = point[2] + axis[2] * noise;
  // std::cout<<returnValue;

  return returnValue;
}

mitk::Point3D mitk::SurfaceModifier::PerturbePoint(
  mitk::Point3D point, double varianceX, double varianceY, double varianceZ)
{
  if (m_myRandomGenerator.IsNull())
    m_myRandomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
  mitk::Point3D returnValue;
  returnValue[0] = point[0] + m_myRandomGenerator->GetNormalVariate(0.0, varianceX);
  returnValue[1] = point[1] + m_myRandomGenerator->GetNormalVariate(0.0, varianceY);
  returnValue[2] = point[2] + m_myRandomGenerator->GetNormalVariate(0.0, varianceZ);
  return returnValue;
}

bool mitk::SurfaceModifier::TransformSurface(mitk::Surface::Pointer surface,
                                             itk::Matrix<double, 3, 3> TransformationR,
                                             itk::Vector<double, 3> TransformationT)
{
  // apply transformation
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->ShallowCopy(surface->GetVtkPolyData()->GetPoints());

  for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
  {
    double p[3];
    points->GetPoint(i, p);
    mitk::Point3D point;
    point[0] = p[0];
    point[1] = p[1];
    point[2] = p[2];
    point = TransformPoint(point, TransformationR, TransformationT);
    p[0] = point[0];
    p[1] = point[1];
    p[2] = point[2];
    points->SetPoint(i, p);
  }
  surface->GetVtkPolyData()->SetPoints(points);

  return true;
}

bool mitk::SurfaceModifier::TransformSurfaceCoGCoordinates(mitk::Surface::Pointer surface,
                                                           itk::Matrix<double, 3, 3> TransformationR,
                                                           itk::Vector<double, 3> TransformationT,
                                                           itk::Matrix<double, 3, 3> &OverallTransformationR,
                                                           itk::Vector<double, 3> &OverallTransformationT)
{
  // initialize return values
  OverallTransformationR.SetIdentity();
  OverallTransformationT.Fill(0);

  // move surface to center of gravity and store transformation
  itk::Matrix<double, 3, 3> TransformRToCenter;
  itk::Vector<double, 3> TransformTToCenter;
  MoveSurfaceToCenter(surface, TransformRToCenter, TransformTToCenter);

  OverallTransformationR = TransformRToCenter;
  OverallTransformationT = TransformTToCenter;

  // apply transformation
  TransformSurface(surface, TransformationR, TransformationT);

  OverallTransformationR = TransformationR * OverallTransformationR;
  OverallTransformationT = (TransformationR * OverallTransformationT) + TransformationT;

  // move surface back to original position (build inverse transformation andy apply it)
  TransformRToCenter = TransformRToCenter.GetInverse();
  TransformTToCenter = (TransformRToCenter * TransformTToCenter) * -1.;
  TransformSurface(surface, TransformRToCenter, TransformTToCenter);

  OverallTransformationR = TransformRToCenter * OverallTransformationR;
  OverallTransformationT = (TransformRToCenter * OverallTransformationT) + TransformTToCenter;

  return true;
}

mitk::Point3D mitk::SurfaceModifier::TransformPoint(mitk::Point3D point,
                                                    itk::Matrix<double, 3, 3> TransformationR,
                                                    itk::Vector<double, 3> TransformationT)
{
  mitk::Point3D returnValue = TransformationR * point + TransformationT;
  return returnValue;
}

bool mitk::SurfaceModifier::AddOutlierToSurface(
  mitk::Surface::Pointer surface, double varianceX, double varianceY, double varianceZ, double outlierChance)
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->ShallowCopy(surface->GetVtkPolyData()->GetPoints());
  for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
  {
    double p[3];
    points->GetPoint(i, p);
    mitk::Point3D point;
    point[0] = p[0];
    point[1] = p[1];
    point[2] = p[2];
    if ((outlierChance - vtkMath::Random(0, 1)) > 0)
      point = PerturbePoint(point, varianceX, varianceY, varianceZ);
    p[0] = point[0];
    p[1] = point[1];
    p[2] = point[2];
    points->SetPoint(i, p);
  }
  surface->GetVtkPolyData()->SetPoints(points);
  return true;
}

bool mitk::SurfaceModifier::PerturbeSurface(
  mitk::Surface::Pointer surface, double varianceX, double varianceY, double varianceZ, double)
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->ShallowCopy(surface->GetVtkPolyData()->GetPoints());
  for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
  {
    double p[3];
    points->GetPoint(i, p);
    mitk::Point3D point;
    point[0] = p[0];
    point[1] = p[1];
    point[2] = p[2];
    point = PerturbePoint(point, varianceX, varianceY, varianceZ);
    p[0] = point[0];
    p[1] = point[1];
    p[2] = point[2];
    points->SetPoint(i, p);
  }
  surface->GetVtkPolyData()->SetPoints(points);

  return true;
}

bool mitk::SurfaceModifier::MoveSurfaceToCenter(mitk::Surface::Pointer surface)
{
  itk::Matrix<double, 3, 3> dummyR;
  itk::Vector<double, 3> dummyT;
  return MoveSurfaceToCenter(surface, dummyR, dummyT);
}

bool mitk::SurfaceModifier::MoveSurfaceToCenter(mitk::Surface::Pointer surface,
                                                itk::Matrix<double, 3, 3> &TransformR,
                                                itk::Vector<double, 3> &TransformT)
{
  // get center of cravity
  mitk::Point3D CoG = GetCenterOfGravity(surface);

  // initialize transforms
  TransformR.SetIdentity();
  TransformT.Fill(0);
  TransformT[0] = -CoG[0];
  TransformT[1] = -CoG[1];
  TransformT[2] = -CoG[2];

  // apply transform
  return TransformSurface(surface, TransformR, TransformT);
}

mitk::Point3D mitk::SurfaceModifier::GetCenterOfGravity(mitk::Surface::Pointer surface)
{
  // convert surface to point set
  mitk::SurfaceToPointSetFilter::Pointer myConverter = mitk::SurfaceToPointSetFilter::New();
  myConverter->SetInput(surface);
  myConverter->Update();
  mitk::PointSet::Pointer pointSet = myConverter->GetOutput();

  // calculate center of gravity
  mitk::Point3D cog;
  cog.Fill(0);
  for (int i = 0; i < pointSet->GetSize(); i++)
  {
    cog[0] += pointSet->GetPoint(i)[0];
    cog[1] += pointSet->GetPoint(i)[1];
    cog[2] += pointSet->GetPoint(i)[2];
  }
  cog[0] /= pointSet->GetSize();
  cog[1] /= pointSet->GetSize();
  cog[2] /= pointSet->GetSize();

  return cog;
}

mitk::Surface::Pointer mitk::SurfaceModifier::DeepCopy(mitk::Surface::Pointer originalSurface)
{
  mitk::Surface::Pointer clonedSurface = mitk::Surface::New();
  vtkSmartPointer<vtkPolyData> clonedPolyData = vtkSmartPointer<vtkPolyData>::New();
  clonedPolyData->DeepCopy(originalSurface->GetVtkPolyData());
  clonedSurface->SetVtkPolyData(clonedPolyData);
  return clonedSurface;
}
