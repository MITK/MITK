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

#include "mitkUSTargetPlacementQualityCalculator.h"

#include "mitkDataNode.h"
#include "mitkSurface.h"
#include "mitkPointSet.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkCenterOfMass.h>
#include <vtkMath.h>
#include <vtkLinearTransform.h>

mitk::USTargetPlacementQualityCalculator::USTargetPlacementQualityCalculator()
: m_OptimalAngle(-1), m_CentersOfMassDistance(-1), m_MeanAngleDifference(-1)
{
}

mitk::USTargetPlacementQualityCalculator::~USTargetPlacementQualityCalculator()
{
}

void mitk::USTargetPlacementQualityCalculator::Update()
{
  if ( m_OptimalAngle < 0 )
  {
    mitkThrow() << "Optimal angle must be set before call of update.";
  }

  if ( m_TargetPoints.IsNull() )
  {
    mitkThrow() << "Targets points must be set before call of update.";
  }

  vtkSmartPointer<vtkPolyData> targetSurface = this->GetTransformedPolydata();

  vtkSmartPointer<vtkCenterOfMass> centerOfMass = vtkSmartPointer<vtkCenterOfMass>::New();
  centerOfMass->SetUseScalarsAsWeights(false);
  centerOfMass->SetInputData(targetSurface);
  centerOfMass->Update();

  double targetCenter[3];
  centerOfMass->GetCenter(targetCenter);

  mitk::Point3D targetCenterPoint;
  targetCenterPoint[0] = targetCenter[0];
  targetCenterPoint[1] = targetCenter[1];
  targetCenterPoint[2] = targetCenter[2];

  MITK_INFO("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepPlacementPlanning")
    << "Center of mass of the target calculated as: " << targetCenterPoint;

  vtkSmartPointer<vtkPoints> plannedTargetsPoints = vtkSmartPointer<vtkPoints>::New();
  std::vector< itk::Vector<float, 3u> > targetsToCenterOfMassVectors;

  for ( mitk::PointSet::PointsConstIterator it = m_TargetPoints->Begin();
    it != m_TargetPoints->End(); ++it )
  {
    // insert all planned target points into a vtkPolyData for calculating
    // the center of mass afterwards
    //mitk::Point3D origin = it->GetData()->GetGeometry()->GetOrigin();
    mitk::Point3D origin = it->Value();
    plannedTargetsPoints->InsertNextPoint(origin[0], origin[1], origin[2]);

    // define normalised vectors from each planned target point to the center
    // of mass of the target structure
    itk::Vector<float, 3u> vector = targetCenterPoint - origin;
    vector.Normalize();
    targetsToCenterOfMassVectors.push_back(vector);
  }

  vtkSmartPointer<vtkPolyData> plannedTargetsPointSet = vtkSmartPointer<vtkPolyData>::New();
  plannedTargetsPointSet->SetPoints(plannedTargetsPoints);

  centerOfMass->SetInputData(plannedTargetsPointSet);
  centerOfMass->Update();
  double plannedCenter[3];
  centerOfMass->GetCenter(plannedCenter);

  mitk::Point3D plannedTargetsCenterPoint;
  plannedTargetsCenterPoint[0] = plannedCenter[0];
  plannedTargetsCenterPoint[1] = plannedCenter[1];
  plannedTargetsCenterPoint[2] = plannedCenter[2];

  MITK_INFO("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepPlacementPlanning")
    << "Center of mass of the planned target points caclulated as: " << plannedTargetsCenterPoint;

  m_CentersOfMassDistance = targetCenterPoint.EuclideanDistanceTo(plannedTargetsCenterPoint);
  MITK_INFO("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepPlacementPlanning")
    << "Distance between target center of mass and planned target points center of mass: " << m_CentersOfMassDistance;

  unsigned int numberOfAngles = targetsToCenterOfMassVectors.size() * (targetsToCenterOfMassVectors.size()-1) / 2;

  m_AngleDifferences.set_size(numberOfAngles);
  unsigned int n = 0;

  m_MeanAngleDifference = 0;
  for ( unsigned int i = 0; i < targetsToCenterOfMassVectors.size(); i++ )
  {
    for ( unsigned int j = i+1; j < targetsToCenterOfMassVectors.size(); j++ )
    {
      double angleDiff = acos ( targetsToCenterOfMassVectors.at(i) * targetsToCenterOfMassVectors.at(j) ) - m_OptimalAngle;
      if ( angleDiff < 0 ) { angleDiff = -angleDiff; }

      m_AngleDifferences.put(n++, angleDiff);
      m_MeanAngleDifference += angleDiff;
    }
  }

  m_MeanAngleDifference /= numberOfAngles; // calculate mean
  m_MeanAngleDifference = vtkMath::DegreesFromRadians(m_MeanAngleDifference);

  MITK_INFO("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepPlacementPlanning")
    << "Angles between the target points: " << m_AngleDifferences;
  MITK_INFO("QmitkUSAbstractNavigationStep")("QmitkUSNavigationStepPlacementPlanning")
    << "Mean difference of angles between the target points: " << m_MeanAngleDifference;
}

vtkSmartPointer<vtkPolyData> mitk::USTargetPlacementQualityCalculator::GetTransformedPolydata()
{
  if ( m_TargetSurface.IsNull() || ! m_TargetSurface->GetVtkPolyData() )
  {
    mitkThrow() << "Target surface must not be null.";
  }

  // transform vtk polydata according to mitk geometry
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(0, m_TargetSurface->GetVtkPolyData());
  transformFilter->SetTransform(m_TargetSurface->GetGeometry()->GetVtkTransform());
  transformFilter->Update();

  return transformFilter->GetOutput();
}
