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
#include "mitkUSNavigationTargetOcclusionFilter.h"
#include "mitkDataNode.h"
#include "mitkSurface.h"

// VTK
#include "vtkSmartPointer.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkOBBTree.h"
#include "vtkFloatArray.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"


mitk::USNavigationTargetOcclusionFilter::USNavigationTargetOcclusionFilter()
  : m_StartPositionInput(0)
{
}

mitk::USNavigationTargetOcclusionFilter::~USNavigationTargetOcclusionFilter()
{
}

void mitk::USNavigationTargetOcclusionFilter::SetTargetStructure(itk::SmartPointer<DataNode> targetStructure)
{
  m_TargetStructure = targetStructure;
}

void mitk::USNavigationTargetOcclusionFilter::SetObstacleStructures(DataStorage::SetOfObjects::ConstPointer obstacleStructures)
{
  m_ObstacleStructures = obstacleStructures;
}

void mitk::USNavigationTargetOcclusionFilter::SelectStartPositionInput(unsigned int n)
{
  m_StartPositionInput = n;
}

void mitk::USNavigationTargetOcclusionFilter::GenerateData()
{
  mitk::NavigationDataPassThroughFilter::GenerateData();

  // get vtk surface an the points
  vtkSmartPointer<vtkPolyData> targetSurfaceVtk = this->GetVtkPolyDataOfTarget();
  if ( ! targetSurfaceVtk ) { return; } // cannot do anything without a target surface

  vtkIdType numberOfPoints = targetSurfaceVtk->GetNumberOfPoints();

  // create array for scalar values
  vtkSmartPointer<vtkFloatArray> colors =
      vtkSmartPointer<vtkFloatArray>::New();
  colors->SetNumberOfComponents(1);
  colors->SetName("USNavigation::Occlusion");

  const mitk::NavigationData* nd = this->GetInput(m_StartPositionInput);

  // set every value to -1 if there is no (valid) navigation data
  if ( nd == 0 || ! nd->IsDataValid() )
  {
    float intersection = -1;
    for ( vtkIdType n = 0; n < numberOfPoints; n++ )
    {
      colors->InsertNextTuple1(intersection);
    }

    if ( numberOfPoints > 0 )
    {
      targetSurfaceVtk->GetPointData()->AddArray(colors);
      targetSurfaceVtk->GetPointData()->Update();
    }

    return;
  }

  // initialize values with 0 (no intersection) if there is valid navigation
  // data and there are some obstacle structures defined
  else if ( m_ObstacleStructures.IsNull() )
  {
    float intersection = 0;
    for ( vtkIdType n = 0; n < numberOfPoints; n++ )
    {
      colors->InsertNextTuple1(intersection);
    }

    if ( numberOfPoints > 0 )
    {
      targetSurfaceVtk->GetPointData()->AddArray(colors);
      targetSurfaceVtk->GetPointData()->Update();
    }

    return;
  }

  // get the current position from the navigation data
  mitk::Point3D position = nd->GetPosition();
  double point1[3];
  point1[0] = position[0]; point1[1] = position[1]; point1[2] = position[2];

  // transform vtk polydata according to mitk geometry
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(0, targetSurfaceVtk);
  transformFilter->SetTransform(m_TargetStructure->GetData()->GetGeometry()->GetVtkTransform());
  transformFilter->Update();
  vtkSmartPointer<vtkPolyData> targetSurfaceVtkTransformed = transformFilter->GetOutput();

  std::vector<bool> occlusion(numberOfPoints, false);

  // calculate occlusion for every obstacle structure
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = m_ObstacleStructures->Begin();
    it != m_ObstacleStructures->End(); ++it)
  {
    vtkSmartPointer<vtkPolyData> polyData = dynamic_cast<mitk::Surface*>(it->Value()->GetData())->GetVtkPolyData();

    // transform the obstacle strucutre according to the mitk geometry
    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
      vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(0, polyData);
    transformFilter->SetTransform(it->Value()->GetData()->GetGeometry()->GetVtkTransform());
    transformFilter->Update();
    polyData = transformFilter->GetOutput();

    // build an obb tree locator
    vtkSmartPointer<vtkOBBTree> cellLocator = vtkSmartPointer<vtkOBBTree>::New();
    cellLocator->SetDataSet(polyData);
    cellLocator->BuildLocator();

    // test for intersection with every point on the surface
    for ( vtkIdType n = 0; n < numberOfPoints; n++ )
    {
      vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      if ( cellLocator->IntersectWithLine(point1, targetSurfaceVtkTransformed->GetPoint(n), points, nullptr) != 0 )
      {
        occlusion.at(n) = true;
      }
    }
  }

  if ( numberOfPoints > 0 )
  {
    // set the occlusion values as scalar array to the vtkPolyData
    float one = 1.0f; float zero = 0.0f;
    for ( std::vector<bool>::iterator it = occlusion.begin(); it != occlusion.end(); ++it )
    {
      colors->InsertNextTuple1(*it ? one : zero);
    }

    targetSurfaceVtk->GetPointData()->AddArray(colors);
    targetSurfaceVtk->GetPointData()->Update();
  }
}

vtkSmartPointer<vtkPolyData> mitk::USNavigationTargetOcclusionFilter::GetVtkPolyDataOfTarget()
{
  if ( m_TargetStructure.IsNull() )
  {
    MITK_WARN("USNavigationTargetOcclusionFilter") << "Target structure must not be null.";
    return nullptr;
  }

  mitk::Surface::Pointer targetSurface = dynamic_cast<mitk::Surface*>(m_TargetStructure->GetData());
  if ( targetSurface.IsNull() )
  {
    MITK_WARN("USNavigationTargetOcclusionFilter") << "A mitk::Surface has to be set to the data node.";
    return nullptr;
  }

  vtkSmartPointer<vtkPolyData> targetSurfaceVtk = targetSurface->GetVtkPolyData();
  if( targetSurfaceVtk == 0 )
  {
    MITK_WARN("USNavigationTargetOcclusionFilter") << "VtkPolyData of the mitk::Surface of the data node must not be null.";
    return nullptr;
  }

  return targetSurfaceVtk;
}
